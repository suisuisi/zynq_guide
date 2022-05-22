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
#include "Xscugic.h"
#include "Xil_exception.h"
#include "xttcps.h"

#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

typedef struct {
	u32 OutputHz;	/* Output frequency */
	u16 Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

static TmrCntrSetup SettingsTable[1] = {
	{10, 0, 0, 0},	/* Ticker timer counter initial setup, only output freq */
};

static XScuGic Intc; //GIC

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
static void TickHandler(void *CallBackRef);


//XTtcPs *TtcPsInst;

int main()
{
	XTtcPs_Config *Config;
	XTtcPs Timer;
	TmrCntrSetup *TimerSetup;
	u16 interval;
	u8 pre;

	init_platform();

	TimerSetup = &SettingsTable[TTC_DEVICE_ID];

	//Timer = &(TtcPsInst[TTC_DEVICE_ID]);
	XTtcPs_Stop(&Timer);



 	printf("\n\rAdam Edition MicroZed Using Vivado \n\r");

 	//initialise the timer
 	Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
 	XTtcPs_CfgInitialize(&Timer, Config, Config->BaseAddress);

 	TimerSetup->Options |= (XTTCPS_OPTION_INTERVAL_MODE |XTTCPS_OPTION_MATCH_MODE|
 						      XTTCPS_OPTION_WAVE_DISABLE);

 	XTtcPs_SetOptions(&Timer, TimerSetup->Options);
 	XTtcPs_CalcIntervalFromFreq(&Timer, TimerSetup->OutputHz,&(TimerSetup->Interval), &(TimerSetup->Prescaler));

    XTtcPs_SetInterval(&Timer, TimerSetup->Interval);
    XTtcPs_SetPrescaler(&Timer, TimerSetup->Prescaler);

    SetupInterruptSystem(&Intc, &Timer);

    interval =  XTtcPs_GetInterval(&Timer);
    printf("%d\r\n",interval);
    pre = XTtcPs_GetPrescaler(&Timer);
    printf("%d\r\n",pre);


    XTtcPs_SetMatchValue(&Timer, 0, (interval/3));
  //  XTtcPs_SetMatchValue(&Timer, u8 MatchIndex, u16 Value);
 //   XTtcPs_SetMatchValue(&Timer, u8 MatchIndex, u16 Value);

     while(1){


     }

     return 0;
}

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt)
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

		XScuGic_Connect(GicInstancePtr, TTC_INTR_ID,
				(Xil_ExceptionHandler)TickHandler, (void *)TtcPsInt);


		XScuGic_Enable(GicInstancePtr, TTC_INTR_ID);
		XTtcPs_EnableInterrupts(TtcPsInt, XTTCPS_IXR_INTERVAL_MASK);
		XTtcPs_EnableInterrupts(TtcPsInt, XTTCPS_IXR_MATCH_0_MASK);
		XTtcPs_Start(TtcPsInt);

		// Enable interrupts in the Processor.
		Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	}

static void TickHandler(void *CallBackRef)
{
	u32 StatusEvent;

	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

	if (0 != (XTTCPS_IXR_INTERVAL_MASK & StatusEvent)) {
		printf("interval interrupt event\n\r");
		//XTtcPs_SetMatchValue(Timer, 0, *MatchReg);
		}
	if (0 != (XTTCPS_IXR_MATCH_0_MASK & StatusEvent)) {
			printf("match interrupt event\n\r");
			//XTtcPs_SetMatchValue(Timer, 0, *MatchReg);
			}

}
