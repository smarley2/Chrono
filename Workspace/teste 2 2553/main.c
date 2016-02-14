#include <msp430.h>

unsigned short ponto2 = 0, a = 0;
float vel_ms = 0, vel_fps = 0;

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

  P2OUT =  0x18;                            // P2.3 and P2.4 set, else reset
  P2REN |= 0x18;                            // P2.3 and P2.4 pullup
  P2IE |= 0x18;                             // P2.3 and P2.4 interrupt enabled
  P2IES |= 0x18;                            // P2.3 and P2.4 Hi/lo edge
  P2IFG &= ~0x18;                           // P2.3 and P2.4 IFG cleared

  TACTL = TASSEL_2 + MC_2;           // SMCLK, contmode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM4 w/interrupt

  while(1)
  {
	  if(a==1)
	  {
//		  vel_ms = 0.5 / (ponto2 * (1/16000000));
//		  vel_fps = vel_ms / 0.3048;
		  a = 0;
	  }
  }

}

// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	if(P2IFG & 0x8) //Interrupção pino 2.3, ponto1
	{
		TA0R = 0;
		P2IFG &= ~0x8;
	}
	if(P2IFG & 0x18) //Interrupção pino 2.4, ponto2
	{
		ponto2 = TA0R;
		a = 1;
		P2IFG &= ~0x18;
	}
}



// Port 1 interrupt service routine
/*#pragma vector=PORT2_VECTOR
__interrupt void Port_1(void)
{
	if(a==0)
	{
		TA0R = 0;
		a = 1;
	}
	else
	{
		ponto2 = TA0R;
		a = 0;
	}

//  P1OUT ^= 0x40;                            // P1.0 = toggle
  P1IFG &= ~0x8;                           // P1.3 IFG cleared
}*/
