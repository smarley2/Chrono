// Header
// Author : Manpreet Singh Minhas
// This file is for 4 bit mode LCD interfacing with msp430g2553 chip
// 16x2 LCD is used

/*
Connections
P1.0 - D4 Pin11
P1.1 - D5 Pin12
P1.2 - D6 Pin13
P1.3 - D7 Pin14
P1.4 - RS Pin4
P1.5 - R/W Pin5
P1.6 - E Pin6

    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_0, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_1, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_2, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_3, GPIO_3_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_3, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_4, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_5, GPIO_5_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_5, GPIO_Direction_Output);
	GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_6, GPIO_Direction_Output);
*/

/*
CodeHex) Command to LCD Instruction Register

1        Clear display screen
2        Return Home
4        Decrement cursor (shift cursor to left)
5        Increment cursor (shift cursor to right)
6        shift display right
7        shift display left
8        Display off, cursor off
A        Display off, cursor on
C        Display on, cursor off
E        Display on, cursor blinking
F        Display on, cursor blinking
10        Shift cursor position to left
14        Shift cursor position to right
18        Shift the entire display to the left
1C        Shift the entire display to the right
80        Force cursor to the beginning of 1st line
C0        Force cursor to the beginning of 2nd line
38        2 lines and 5 x 7 matrix

For displaying the characters on the 1st line send the command 0x80
For displaying the characters on the 2nd line send the command 0xC0
For displaying the characters on the 3rd line send the command 0x94
For displaying the characters on the 4th line send the command 0xD4
*/
//#include <msp430g2553.h>
//#include <msp430.h>

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#include "f2802x_common/include/gpio.h"

GPIO_Handle myGpio;

#define DR GPIO_setHigh(myGpio,GPIO_Number_4); // define RS high
#define CWR GPIO_setLow(myGpio,GPIO_Number_4); // define RS low
#define READ GPIO_setHigh(myGpio,GPIO_Number_5); // define Read signal R/W = 1 for reading
#define WRITE GPIO_setLow(myGpio,GPIO_Number_5); // define Write signal R/W = 0 for writing
#define ENABLE_HIGH GPIO_setHigh(myGpio,GPIO_Number_32); // define Enable high signal
#define ENABLE_LOW GPIO_setLow(myGpio,GPIO_Number_32); // define Enable Low signal

unsigned volatile long i;
unsigned volatile long j;

void delay(unsigned int k)
{
for(j=0;j<=k;j++)
{
for(i=0;i<=100;i++)
	{}
}
}

void data_write(void)
{
ENABLE_HIGH;
delay(20);
ENABLE_LOW;
}

void data_read(void)
{
ENABLE_LOW;
delay(20);
ENABLE_HIGH;
}

void check_busy(void)
{
	CWR;
	READ;
	GPIO_setDirection(myGpio, GPIO_Number_3, GPIO_Direction_Input);
	while((GPIO_getData(myGpio, GPIO_Number_3))==1)
	{
	data_read();
	}
	GPIO_setDirection(myGpio, GPIO_Number_3, GPIO_Direction_Output);

}

void send_command(unsigned char cmd)
{
check_busy();
WRITE;
CWR;
GpioDataRegs.GPADAT.all = (GpioDataRegs.GPADAT.all & 0xFFF0)|((cmd>>4) & 0x000F); // send higher nibble
data_write(); // give enable trigger
GpioDataRegs.GPADAT.all = (GpioDataRegs.GPADAT.all & 0xFFF0)|(cmd & 0x000F); // send lower nibble
data_write(); // give enable trigger
}

void send_data(unsigned char data)
{
check_busy();
WRITE;
DR;
GpioDataRegs.GPADAT.all = (GpioDataRegs.GPADAT.all & 0xFFF0)|((data>>4) & 0x000F); // send higher nibble
data_write(); // give enable trigger
GpioDataRegs.GPADAT.all = (GpioDataRegs.GPADAT.all & 0xFFF0)|(data & 0x000F); // send lower nibble
data_write(); // give enable trigger
}

void send_string(char *s)
{
while(*s)
{
send_data(*s);
s++;
}
}

void lcd_init(void)
{
send_command(0x33);
send_command(0x32);
send_command(0x28); // 4 bit mode
send_command(0x0E); // clear the screen
send_command(0x01); // display on cursor on
send_command(0x06); // increment cursor
send_command(0x80); // row 1 column 1
}

void integerToLcd(long integer )
{

unsigned char thousands = 0,hundreds = 0,tens = 0,ones = 0;

	while(integer > 9999){
		integer = integer / 10;
	}

	if(integer>999)
	{
		thousands = integer / 1000;
		send_data(thousands + 0x30);
	}

	if(integer>99)
	{
	hundreds = ((integer - thousands*1000)) / 100;
	send_data( hundreds + 0x30);
	}

	if(integer>9)
	{
	tens=(integer%100)/10;
	send_data( tens + 0x30);
	}

	ones=integer%10;
	send_data( ones + 0x30);
}

void integerToLcd_resto(long integer_resto )
{
unsigned char hundreds = 0,tens = 0,ones = 0;

	while(integer_resto > 999){
		integer_resto = integer_resto / 10;
	}

	hundreds = integer_resto / 100;
	send_data( hundreds + 0x30);

	tens=(integer_resto%100)/10;
	send_data( tens + 0x30);

	ones=integer_resto%10;
	send_data( ones + 0x30);
}
