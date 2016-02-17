#include <msp430.h>
#include <lcd.h>

unsigned short ponto2 = 0;
unsigned short volatile estado = 0;
unsigned long vel_ms = 0, vel_fps = 0;
volatile int temp = 0;

void pin_init();
void tempInit();
int tempOut();

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer


  if (CALBC1_16MHZ==0xFF)					// If calibration constants erased
  {
    while(1);                               // do not load, trap CPU!!
  }
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;                   // Set range
  BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;                   // Set range
  DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation

/*  BCSCTL1 |= DIVA_0;
  BCSCTL1 &= ~XT2OFF;
  BCSCTL2 = SELM_2 + DIVM_0 + SELS + DIVS_0;
  BCSCTL3 = XT2S_2 + XT2S1 + XCAP_3;
*/
  pin_init();
  lcd_init();
  tempInit();//initialise adc

  TACTL = ID_0 + TASSEL_2 + MC_2;           // SMCLK, contmode

  __bis_SR_register(GIE);

  while(1)
  {
	  if(estado == 2)
	  {
//		  vel_ms = 0.5 / (ponto2 * (1/16000000));
//		  vel_fps = vel_ms / 0.3048;
//		  ponto2 = 26246; //utilizado para teste

		  vel_ms = 8000000 / ponto2; // 1/f * distância / clks
		  vel_fps = 26246719 / ponto2; // 1/feet * 1/f * distância / clks
//		  send_data(0x01);
		  send_command(0x0E);
		  send_command(0x80);
		  send_string("Vel = ");
		  integerToLcd(vel_ms);
		  send_string(" m/s");
		  send_command(0xC0);
		  send_string("Vel = ");
		  integerToLcd(vel_fps);
		  send_string(" fps");
		  temp=tempOut();
		  send_command(0x94);
		  send_string("Temp = ");
		  integerToLcd(temp);
		  send_string(" °C");

		  estado = 0;
	  }
  }

}

// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	if((P2IFG & 0x08) && (estado == 0)) //Interrupção pino 2.3, ponto1
	{
		TA0R = 0;
		estado = 1;
	}
	if((P2IFG & 0x10) && (estado == 1)) //Interrupção pino 2.4, ponto2
	{
		ponto2 = TA0R;
		estado = 2;
	}

	P2IFG &= ~0x18;
}


void pin_init()
{
/*
PxDIR	Port Px Direction	Clearing a bit configures a pin as an input while setting a bit configures a pin for output.
PxREN	Port Px Resistor Enable	Setting a bit activates an internal pull-up/pull-down resistor on a pin. The corresponding bit in the PxOUT register selects pull-up if set to one or pull-down if set to zero.
PxSEL	Port Px Selection	Clearing a bit configures a pin as a digital input/output (GPIO), while setting a bit selects an alternative function.
PxIE	Port Px Interrupt Enable	Setting a bit enables an interrupt when the input pin changes.
PxIES	Port Px Interrupt Edge Select	Interrupts can be generated on either a positive edge (low to high) or negative edge (high to low) change of an input pin. 1 para Hi/Lo e 0 para Lo/Hi
PxIFG	Port Px Interrupt Flag	A bit is set when the selected transition has been detected on the input.
PxIN can then be read
 */
	// Configuração do LCD
	  P1DIR |= 0x3F; //0,1,2,3,4,5 utilizados como outpu no LCD.
	  P1OUT &= 0x00; //Desliga todas as saídas da porta 1.

	  P2DIR |= 0x01; //0 utilizado no LCD
	  P2OUT &= 0x00; //Desliga todos os pinos da porta 2.
	  ///////////////////////////////////////////////////

	  //Configuração das entradas
	  P2OUT =  0x18;                            // P2.3 and P2.4 set, else reset
//	  P2REN |= 0x18;                            // P2.3 and P2.4 pullup
	  P2IE |= 0x18;                             // P2.3 and P2.4 interrupt enabled
	  P2IES &= 0x00;                            // P2.3 and P2.4 Lo/Hi edge
	  P2IFG &= ~0x18;                           // P2.3 and P2.4 IFG cleared
      ///////////////////////////////////////
}

void tempInit()
{
    ADC10CTL0=SREF_1 + REFON + ADC10ON + ADC10SHT_3 ; //1.5V ref,Ref on,64 clocks for sample
    ADC10CTL1=INCH_10+ ADC10DIV_3; //temp sensor is at 10 and clock/4
}
int tempOut()
{
    int t=0;
    __delay_cycles(1000);              //wait 4 ref to settle
    ADC10CTL0 |= ENC + ADC10SC;      //enable conversion and start conversion
    while(ADC10CTL1 & BUSY);         //wait..i am converting..pum..pum..
    t=ADC10MEM;                       //store val in t    ADC10CTL0 |= ENC + ADC10SC;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    ADC10CTL0 |= ADC10SC;
    while(ADC10CTL1 & BUSY);
    t+=ADC10MEM;
    t = t / 10;

    ADC10CTL0&=~ENC;                     //disable adc conv
    return(int) ((t * 27069L - 18169625L) >> 16); //convert and pass
}
