//#############################################################################
//
//  File:   Example_F2802xCpuTimer.c
//
//  Title:  F2802x CPU Timer Example
//
//! \addtogroup example_list
//!  <h1>CPU Timer</h1>
//!
//!   This example configures CPU Timer0, 1, & 2 and increments
//!   a counter each time the timer asserts an interrupt.
//!
//!   Watch Variables:
//!   - timer0IntCount
//!   - timer1IntCount
//!   - timer2IntCount
//
//#############################################################################
// $TI Release: F2802x Support Library v230 $
// $Release Date: Fri May  8 07:43:05 CDT 2015 $
// $Copyright: Copyright (C) 2008-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//#############################################################################

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#include "f2802x_common/include/clk.h"
#include "f2802x_common/include/flash.h"
#include "f2802x_common/include/gpio.h"
#include "f2802x_common/include/pie.h"
#include "f2802x_common/include/pll.h"
#include "f2802x_common/include/timer.h"
#include "f2802x_common/include/wdog.h"

#include "lcd.h"

#define start_button GPIO_Number_34

void wait_start(void);
void escreve_shoot(void);
void Adc_init(void);
void temp_interna(void);
void Int_init(void);
void Gpio_init(void);
void Timer_init(void);
void Clock_init(void);

// Prototype statements for functions found within this file.
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);
__interrupt void xint1_isr(void);
__interrupt void xint2_isr(void);

unsigned long timer0IntCount;
unsigned long timer1IntCount;
unsigned long timer2IntCount;

unsigned long ponto2 = 0, temperatura = 0;
unsigned long volatile estado = 3;
unsigned long vel_ms = 0, vel_fps = 0, resto_vel_ms = 0, resto_vel_fps = 0;

ADC_Handle myAdc;
CLK_Handle myClk;
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;
TIMER_Handle myTimer0, myTimer1, myTimer2;
CPU_Handle myCpu;
PLL_Handle myPll;
WDOG_Handle myWDog;

void main(void)
{
    // Initialize all the handles needed for this application
    myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myTimer0 = TIMER_init((void *)TIMER0_BASE_ADDR, sizeof(TIMER_Obj));
    myTimer1 = TIMER_init((void *)TIMER1_BASE_ADDR, sizeof(TIMER_Obj));
    myTimer2 = TIMER_init((void *)TIMER2_BASE_ADDR, sizeof(TIMER_Obj));
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));

    timer0IntCount = 0;
    timer1IntCount = 0;
    timer2IntCount = 0;

    //Inicializa Clock
    Clock_init();

    // Inicialização dos GPIO
    Gpio_init();
	
    // Inicialização do Adc para temp interna
    Adc_init();

    // Inicialização dos timers
    Timer_init();

    // Inicializa as interrupções
    Int_init();

    lcd_init();
	send_command(0x01); // Limpa a tela
	send_command(0x80); // Primeira linha
	send_string("Press Start");
	
  while(1)
  {
	  if(estado == 3)
	  {
		  wait_start();
	  }

	  if(estado == 2)
	  {
		  escreve_shoot();
	  }
  }	
	
}

__interrupt void cpu_timer1_isr(void)
{
//    GPIO_toggle(myGpio, GPIO_Number_2);
//  timer1IntCount = CpuTimer0Regs.PRD.all;
//	timer0IntCount = CpuTimer0Regs.TIM.all;
//	CpuTimer0Regs.TCR.bit.TRB = 1;
//	TIMER_reload(myTimer0);
    PIE_clearInt(myPie, PIE_GroupNumber_1);
}

__interrupt void cpu_timer2_isr(void)
{
   // timer2IntCount++;
}


__interrupt void xint1_isr(void)
{

	if(estado == 0) //Interrupção pino 2.3, ponto1
	{
		//	TIMER_reload(myTimer0);
		CpuTimer0Regs.TCR.bit.TRB = 1;
		estado = 1;
	}

    // Acknowledge this interrupt to get more from group 1
    PIE_clearInt(myPie, PIE_GroupNumber_1);
}

__interrupt void xint2_isr(void)
{
	if(estado == 1) //Interrupção pino 2.4, ponto2
	{
		ponto2 = CpuTimer0Regs.TIM.all;;
		estado = 2;
	}	

    // Acknowledge this interrupt to get more from group 1
    PIE_clearInt(myPie, PIE_GroupNumber_1);
}

void wait_start()
{
	 if(GPIO_getData(myGpio, start_button) == 0)
		{
			while(GPIO_getData(myGpio, start_button)==0){}
			send_command(0x01); // Limpa a tela
			send_command(0x80); // Primeira linha
			send_string("Waiting Shoot");
			estado = 0;
		}
}

void escreve_shoot()
{
	//		  vel_ms = 0.5 / (ponto2 * (1/50000000));
	//		  vel_fps = vel_ms / 0.3048;
			  ponto2 = (0xFFFFFFFF - ponto2);
			  if(ponto2 > 25000000)
			  {
				  send_command(0x01); // Limpa a tela
				  send_command(0x0E); // Limpa a tela
				  send_command(0x80); // Primeira linha
				  send_string("Too slow, try again");
				  send_command(0xC0); // Segunda Linha
				  send_string("Waiting Start Button");
			  }
			  else
			  {
				  vel_ms = 25000000 / ponto2; // 1/f * distância / clks
				  resto_vel_ms = 25000000 % ponto2; // 1/f * distância / clks
				  vel_fps = 82020997 / ponto2; // 1/feet * 1/f * distância / clks
				  resto_vel_fps = 82020997 % ponto2; // 1/feet * 1/f * distância / clks
				  send_command(0x01); // Limpa a tela
//				  send_command(0x0E); // Limpa a tela
				  send_command(0x80); // Primeira linha
				  send_string("Vel = ");
				  integerToLcd(vel_ms);
				  send_string(".");
				  integerToLcd_resto(resto_vel_ms);
				  send_string(" m/s");
				  send_command(0xC0); // Segunda Linha
				  send_string("Vel = ");
				  integerToLcd(vel_fps);
				  send_string(".");
				  integerToLcd_resto(resto_vel_fps);
				  send_string(" fps");
				  temp_interna();
				  temp_interna();
				  send_command(0x94); // Terceira Linha
				  send_string("Temp = ");
				  integerToLcd(temperatura);
				  send_string(" °C");
				  send_command(0xD4); // Quarta Linha
				  send_string("Waiting Start Button");
			  }

			  estado = 3;	//Waiting Start
}

void temp_interna(void)
{
	long temp_sensor = 0;
	ADC_forceConversion(myAdc, ADC_SocNumber_0);
    ADC_forceConversion(myAdc, ADC_SocNumber_1);
    //Wait for end of conversion.
    while(ADC_getIntStatus(myAdc, ADC_IntNumber_1) == 0){}
    // Clear ADCINT1
    ADC_clearIntFlag(myAdc, ADC_IntNumber_1);

    temp_sensor = ADC_readResult(myAdc, ADC_ResultNumber_4);
    temperatura = ADC_getTemperatureC(myAdc, temp_sensor);
    return;
}

//===========================================================================
// Configurações
//===========================================================================

void Adc_init(void)
{
    ADC_enableBandGap(myAdc);
    ADC_enableRefBuffers(myAdc);
    ADC_powerUp(myAdc);
    ADC_enable(myAdc);
    ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);
    ADC_enableTempSensor(myAdc);                                            //Connect channel A5 internally to the temperature sensor

    //Temperatura interna
    ADC_setSocChanNumber (myAdc, ADC_SocNumber_0, ADC_SocChanNumber_A5);    //Set SOC0 channel select to ADCINA5
    ADC_setSocChanNumber (myAdc, ADC_SocNumber_1, ADC_SocChanNumber_A5);    //Set SOC1 channel select to ADCINA5
    ADC_setSocSampleWindow(myAdc, ADC_SocNumber_0, ADC_SocSampleWindow_37_cycles);   //Set SOC0 acquisition period to 37 ADCCLK
    ADC_setSocSampleWindow(myAdc, ADC_SocNumber_1, ADC_SocSampleWindow_37_cycles);   //Set SOC1 acquisition period to 37 ADCCLK
    ADC_setIntSrc(myAdc, ADC_IntNumber_1, ADC_IntSrc_EOC1);                 //Connect ADCINT1 to EOC1
    ADC_enableInt(myAdc, ADC_IntNumber_1);                                  //Enable ADCINT1

}

void Gpio_init(void)
{
	//GPIO utilizado para botões.
	GPIO_setPullUp(myGpio, GPIO_Number_34, GPIO_PullUp_Enable);   // Disable pullup
	GPIO_setMode(myGpio, GPIO_Number_34, GPIO_34_Mode_GeneralPurpose);  // GPIO34
    GPIO_setDirection(myGpio, GPIO_Number_34, GPIO_Direction_Input);

	// GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);   // Disable pullup
	// GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_GeneralPurpose);  // GPIO34
    // GPIO_setDirection(myGpio, GPIO_Number_2, GPIO_Direction_Output);
	// GPIO_setLow(myGpio,GPIO_Number_2);  // GPIO34 - Output

	//GPIO's utilizados para interrupção
    GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_GeneralPurpose);
    GPIO_setDirection(myGpio, GPIO_Number_28, GPIO_Direction_Input);
    GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_Sync);
    GPIO_setExtInt(myGpio, GPIO_Number_28, CPU_ExtIntNumber_1);
    GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);

    GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_GeneralPurpose);
    GPIO_setDirection(myGpio, GPIO_Number_29, GPIO_Direction_Input);
    GPIO_setQualification(myGpio, GPIO_Number_29, GPIO_Qual_Sync);
    GPIO_setExtInt(myGpio, GPIO_Number_29, CPU_ExtIntNumber_2);
    GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Enable);

	//Configuração de GPIO´s para o LCD
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
	GPIO_setMode(myGpio, GPIO_Number_32, GPIO_32_Mode_GeneralPurpose);
	GPIO_setDirection(myGpio, GPIO_Number_32, GPIO_Direction_Output);

}

void Timer_init(void)
{
	// Step 4. Initialize the Device Peripheral. This function can be
	//         found in f2802x_CpuTimers.c
	    TIMER_stop(myTimer0);
	    TIMER_stop(myTimer1);

	// Configure timer 0 to run free with FFFFFFFF
	// Configure CPU-Timer 1 to interrupt every second:
	// 50MHz CPU Freq, 1 second Period (in uSeconds)

	    TIMER_setPeriod(myTimer0, 0xFFFFFFFF);
	    TIMER_setPeriod(myTimer1, 50 * 1000000);

	    TIMER_setPreScaler(myTimer0, 0);
	    TIMER_reload(myTimer0);
	    TIMER_setEmulationMode(myTimer0, TIMER_EmulationMode_RunFree);

	    TIMER_setPreScaler(myTimer1, 0);
	    TIMER_reload(myTimer1);
	    TIMER_setEmulationMode(myTimer1, TIMER_EmulationMode_StopAfterNextDecrement);
	    TIMER_enableInt(myTimer1);

	//   CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	    TIMER_start(myTimer0);
	//   CpuTimer1Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	    TIMER_start(myTimer1);
	//   CpuTimer2Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	//    TIMER_start(myTimer2);
}

void Int_init()
{
    // Disable the PIE and all interrupts
    PIE_disable(myPie);
    PIE_disableAllInts(myPie);
    CPU_disableGlobalInts(myCpu);
    CPU_clearIntFlags(myCpu);

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in f2802x_DefaultIsr.c.
	// This function is found in f2802x_PieVect.c.
	    PIE_setDebugIntVectorTable(myPie);
	    PIE_enable(myPie);

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	    EALLOW;  // This is needed to write to EALLOW protected registers

//	    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_7, (intVec_t)&cpu_timer0_isr);
	    PIE_registerSystemIntHandler(myPie, PIE_SystemInterrupts_TINT1, (intVec_t)&cpu_timer1_isr);
	    PIE_registerSystemIntHandler(myPie, PIE_SystemInterrupts_TINT2, (intVec_t)&cpu_timer2_isr);

	    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_4,(intVec_t)&xint1_isr);
	    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_5,(intVec_t)&xint2_isr);

	    PIE_enableInt(myPie, PIE_GroupNumber_1, PIE_InterruptSource_XINT_1);
	    PIE_enableInt(myPie, PIE_GroupNumber_1, PIE_InterruptSource_XINT_2);

	    PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_1,PIE_ExtIntPolarity_RisingEdge);
	    PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_2,PIE_ExtIntPolarity_RisingEdge);

	    EDIS;    // This is needed to disable write to EALLOW protected registers

	// Step 5. User specific code, enable interrupts:

	    PIE_enableTimer0Int(myPie);
	    PIE_enableExtInt(myPie, CPU_ExtIntNumber_1);
	    PIE_enableExtInt(myPie, CPU_ExtIntNumber_2);
	// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
	// which is connected to CPU-Timer 1, and CPU int 14, which is connected
	// to CPU-Timer 2:
	    CPU_enableInt(myCpu, CPU_IntNumber_1);
	    CPU_enableInt(myCpu, CPU_IntNumber_13);
	    CPU_enableInt(myCpu, CPU_IntNumber_14);

	// Enable global Interrupts and higher priority real-time debug events:
	    CPU_enableGlobalInts(myCpu); // Enable Global interrupt INTM
	    CPU_enableDebugInt(myCpu); // Enable Global realtime interrupt DBGM
}

void Clock_init()
{
    // Perform basic system initialization
    WDOG_disable(myWDog);
    CLK_enableAdcClock(myClk);
    (*Device_cal)();

    //Select the internal oscillator 1 as the clock source
    CLK_setOscSrc(myClk, CLK_OscSrc_Internal);

    // Setup the PLL for x10 /2 which will yield 50Mhz = 10Mhz * 10 / 2
    PLL_setup(myPll, PLL_Multiplier_10, PLL_DivideSelect_ClkIn_by_2);
}
