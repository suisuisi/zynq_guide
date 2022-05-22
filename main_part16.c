/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 */

#include <stdio.h>
#include "platform.h"
#include "xadcps.h"
#include "xgpiops.h"
#include "xil_types.h"
#include "Xscugic.h"
#include "Xil_exception.h"
#include "xscutimer.h"
#include "xscuwdt.h"
//XADC info
#define XPAR_AXI_XADC_0_DEVICE_ID 0

//GPIO info
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID	XPS_GPIO_INT_ID

//timer info
#define TIMER_DEVICE_ID		XPAR_XSCUTIMER_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_IRPT_INTR		XPAR_SCUTIMER_INTR


//define private watchdog
#define WDT_DEVICE_ID		XPAR_SCUWDT_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define WDT_IRPT_INTR		XPAR_SCUWDT_INTR

#define WDT_LOAD_VALUE		0xFFFFFFFF

#define ledpin 47
#define pbsw 51
#define LED_DELAY     100000000
#define TIMER_LOAD_VALUE	0xFF
//void print(char *str);

//static XAdcPs  XADCMonInst; //XADC
static XScuGic Intc; //GIC
static XGpioPs Gpio; //GPIO
static XScuTimer Timer;//timer
static XScuWdt WdtInstance; //watchdog

static int toggle = 0;//used to toggle the LED

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId,
		XScuTimer *TimerInstancePtr, u16 TimerIntrId,XScuWdt *WdtInstancePtr, u16 WdtIntrId);

static void GPIOIntrHandler(void *CallBackRef, int Bank, u32 Status);
static void TimerIntrHandler(void *CallBackRef);
static void WdtIntrHandler(void *CallBackRef);

int main()
{

//	 XAdcPs_Config *ConfigPtr;           //xadc config
	 XScuTimer_Config *TMRConfigPtr;     //timer config
	 XGpioPs_Config *GPIOConfigPtr;      //gpio config
	 XScuWdt_Config *WCHConfigPtr;		 //watch dog
//	 XAdcPs *XADCInstPtr = &XADCMonInst; //xadc pointer
//	 u32 Timebase = 0;
//	 u32 ExpiredTimeDelta = 0;
	 int reg=0;

	 init_platform();



     //GPIO Initilization
     GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
     XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr,GPIOConfigPtr->BaseAddr);
     //set direction and enable output
     XGpioPs_SetDirectionPin(&Gpio, ledpin, 1);
     XGpioPs_SetOutputEnablePin(&Gpio, ledpin, 1);
    //set direction input pin
     XGpioPs_SetDirectionPin(&Gpio, pbsw, 0x0);

     //timer initialisation
     TMRConfigPtr = XScuTimer_LookupConfig(TIMER_DEVICE_ID);
     XScuTimer_CfgInitialize(&Timer, TMRConfigPtr,TMRConfigPtr->BaseAddr);

     XScuTimer_SelfTest(&Timer);
 	 //load the timer
 	 XScuTimer_LoadTimer(&Timer, TIMER_LOAD_VALUE);
 	 //XScuTimer_EnableAutoReload(&Timer);
 	 //start timer
 	 //XScuTimer_Start(&Timer);

 	 //watchdog
 	WCHConfigPtr = XScuWdt_LookupConfig(WDT_DEVICE_ID);
 	XScuWdt_CfgInitialize(&WdtInstance, WCHConfigPtr,
 					      WCHConfigPtr->BaseAddr);

 	printf("\n\rAdam Edition MicroZed Using Vivado How To Printf \n\r");
 	reg = XScuWdt_IsWdtExpired(&WdtInstance);

 	printf("Previous reset state = %d\n\r", reg);

 	XScuWdt_LoadWdt(&WdtInstance, WDT_LOAD_VALUE);


    //set up the interrupts
    SetupInterruptSystem(&Intc, &Gpio, GPIO_INTERRUPT_ID,&Timer,TIMER_IRPT_INTR,&WdtInstance,WDT_IRPT_INTR);


     while(1){
     }

     return 0;
}

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId,
		XScuTimer *TimerInstancePtr, u16 TimerIntrId, XScuWdt *WdtInstancePtr, u16 WdtIntrId)
{


		XScuGic_Config *IntcConfig; //GIC config
		Xil_ExceptionInit();
		//initialise the GIC
		IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
		XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,
						IntcConfig->CpuBaseAddress);
	    //connect to the hardware
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler)XScuGic_InterruptHandler,
					GicInstancePtr);
	    //set up the GPIO interrupt
		XScuGic_Connect(GicInstancePtr, GpioIntrId,
					(Xil_ExceptionHandler)XGpioPs_IntrHandler,
					(void *)Gpio);
		//set up the timer interrupt
		XScuGic_Connect(GicInstancePtr, TimerIntrId,
						(Xil_ExceptionHandler)TimerIntrHandler,
						(void *)TimerInstancePtr);
		//set up the watchdog
		XScuGic_Connect(GicInstancePtr, WdtIntrId,
						(Xil_ExceptionHandler)WdtIntrHandler,
						(void *)WdtInstancePtr);

		//setup the watchdog
		XScuWdt_SetWdMode(WdtInstancePtr);

		//Enable  interrupts for all the pins in bank 0.
		XGpioPs_SetIntrTypePin(Gpio, pbsw, XGPIOPS_IRQ_TYPE_EDGE_RISING);
		//Set the handler for gpio interrupts.
		XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, GPIOIntrHandler);
		//Enable the GPIO interrupts of Bank 0.
		XGpioPs_IntrEnablePin(Gpio, pbsw);
		//Enable the interrupt for the GPIO device.
		XScuGic_Enable(GicInstancePtr, GpioIntrId);
		//enable the interrupt for the Timer at GIC
		XScuGic_Enable(GicInstancePtr, TimerIntrId);
		//enable interrupt on the timer
		XScuTimer_EnableInterrupt(TimerInstancePtr);
		//enable interrupt on the watchdog
		XScuGic_Enable(GicInstancePtr, WdtIntrId);
		// Enable interrupts in the Processor.
		Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	}

static void TimerIntrHandler(void *CallBackRef)
{

	XScuTimer *TimerInstancePtr = (XScuTimer *) CallBackRef;
	XScuTimer_ClearInterruptStatus(TimerInstancePtr);
	//printf("****Timer Event!!!!!!!!!!!!!****\n\r");
	XScuWdt_Start(&WdtInstance);

}


static void GPIOIntrHandler(void *CallBackRef, int Bank, u32 Status)
{
	int delay;
	XGpioPs *Gpioint = (XGpioPs *)CallBackRef;

	//printf("****button pressed****\n\r");

	if (toggle == 0 )
		toggle = 1;
	else
		toggle = 0;

	//load timer
	XScuTimer_LoadTimer(&Timer, TIMER_LOAD_VALUE);
	//start timer
	XScuTimer_Start(&Timer);
	XGpioPs_WritePin(Gpioint, ledpin, toggle);

	for( delay = 0; delay < LED_DELAY; delay++)//wait
	{}
	XGpioPs_IntrClearPin(Gpioint, pbsw);
}
static void WdtIntrHandler(void *CallBackRef)
{}
