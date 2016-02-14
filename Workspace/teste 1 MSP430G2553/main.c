#include <msp430.h>
#include <lcd.h>

unsigned short ponto1 = 0, ponto1_overflow = 0, ponto2 = 0, ponto2_overflow = 0, overflow = 0, a = 0;
unsigned long diff = 0;

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

  P1DIR = 0x41;                             // P1.6 and P1.0 output, else input

  P1OUT =  0x8;                            // P1.3 set, else reset
  P1REN |= 0x8;                            // P1.3 pullup
  P1IE |= 0x8;                             // P1.3 interrupt enabled
  P1IES |= 0x8;                            // P1.3 Hi/lo edge
  P1IFG &= ~0x8;                           // P1.3 IFG cleared

  //TACTL = TASSEL_2 + MC_2 + TAIE;           // SMCLK, contmode, interrupt
  TACTL = TASSEL_2 + MC_2;           // SMCLK, contmode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM4 w/interrupt


  lcd_init();
  send_string("Manpreet Singh");
  send_command(0xC0);
  send_string("Minhas");

}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(a==0)
	{
		TA0R = 0;
		overflow = 0;
		ponto1_overflow = overflow;
		ponto1 = TA0R;
		a = 1;
	}
	else
	{
		ponto2_overflow = overflow;
		ponto2 = TA0R;

		diff = ((ponto2_overflow - ponto1_overflow)*65535);
		if(diff > 0)
		{
			diff = diff - ponto1 + ponto2;
		}
		else
		{
			diff = ponto2 - ponto1;
		}

		a = 0;
	}


  P1OUT ^= 0x40;                            // P1.0 = toggle
  P1IFG &= ~0x8;                           // P1.3 IFG cleared
}


// Timer_A3 Interrupt Vector (TA0IV) handler
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A(void)
{
 switch( TA0IV )
 {
   case  2: break;                          // CCR1 not used
   case  4: break;                          // CCR2 not used
   case 10:
	   overflow ++;
	   P1OUT ^= 0x01;                  // overflow
       break;
 }
}
