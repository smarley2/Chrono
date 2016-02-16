#define TINYPRINTF_DEFINE_TFP_PRINTF 1
#define TINYPRINTF_DEFINE_TFP_SPRINTF 0
#define TINYPRINTF_OVERRIDE_LIBC 0

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <msp430.h>
#include <lcd.h>
#include <tinyprintf.h>

/* Clear unused warnings for actually unused variables */
#define UNUSED(x) (void)(x)

#define TPRINTF(expr...) \
  ({ printf("libc_printf(%s) -> ", #expr); printf(expr); \
     printf(" tfp_printf(%s) -> ", #expr); tfp_printf(expr); })


unsigned short ponto2 = 0;
unsigned short volatile estado = 0;
unsigned long vel_ms = 0, vel_fps = 0;

void pin_init();

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer


  if (CALBC1_16MHZ==0xFF)					// If calibration constants erased
  {
    while(1);                               // do not load, trap CPU!!
  }
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;                   // Set range
  DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation

  BCSCTL1 = DIVA_0;
  BCSCTL2 = SELM_2 + DIVM_0 + SELS + DIVS_0;
  BCSCTL3 = XT2S_2 + XCAP_3;

  pin_init();
  lcd_init();

  TACTL = TASSEL_2 + MC_2;           // SMCLK, contmode

  __bis_SR_register(GIE);

  while(1)
  {
	  if(estado == 2)
	  {
//		  vel_ms = 0.5 / (ponto2 * (1/16000000));
//		  vel_fps = vel_ms / 0.3048;
		  vel_ms = 8000000 / ponto2; // 1/f * distância / clks
		  vel_fps = 26246719 / ponto2; // 1/feet * 1/f * distância / clks

		  send_string("Vel = ");
		  send_command(0xC0);

		  estado = 0;
	  }
  }

}

// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	if((P2IFG & 0x8) && (estado == 0)) //Interrupção pino 2.3, ponto1
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
	  P2REN |= 0x18;                            // P2.3 and P2.4 pullup
	  P2IE |= 0x18;                             // P2.3 and P2.4 interrupt enabled
	  P2IES &= 0x00;                            // P2.3 and P2.4 Lo/Hi edge
	  P2IFG &= ~0x18;                           // P2.3 and P2.4 IFG cleared
      ///////////////////////////////////////
}
