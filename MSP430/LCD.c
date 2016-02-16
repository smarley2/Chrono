//Main Code:
/* LCD_own.c
* Created on: 12-Nov-2013
* Author: Manpreet
* In this program we interface the lcd in 4 bit mode. We send strings and display it on the screen.
*/
#include "lcd.h"
void main(void)
{
WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer
lcd_init();
send_string("Manpreet Singh");
send_command(0xC0);
send_string("Minhas");
while(1){}
}
