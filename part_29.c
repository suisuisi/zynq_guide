#include <stdio.h>
#include "platform.h"
#include "xscugic.h"
#include "xdmaps.h"
#include "xil_exception.h"
#include "xil_types.h"
#include "xil_printf.h"

#define DMA_DEVICE_ID 			XPAR_XDMAPS_1_DEVICE_ID
#define INTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define DMA_FAULT_INTR			XPAR_XDMAPS_0_FAULT_INTR
#define DMA_DONE_INTR_0			XPAR_XDMAPS_0_DONE_INTR_0
#define DMA_LENGTH	1024


void XDma_Config(u16 DeviceId);
void SetupInterrupt(XScuGic *GicPtr, XDmaPs *DmaPtr);
void DmaISR(unsigned int Channel, XDmaPs_Cmd *DmaCmd,void *CallbackRef);

static int Src[DMA_LENGTH];
static int Dst[DMA_LENGTH];

XDmaPs DmaInstance;
XScuGic GicInstance;

int main()
{



	init_platform();

 	printf("\n\rAdam Edition MicroZed Using Vivado \n\r");

 	XDma_Config(DMA_DEVICE_ID);



    return 0;

}

void XDma_Config(u16 DeviceId)
{
		int Index;
		unsigned int Channel = 0;
//		int Status;
//		int TestStatus;
//		int TestRound;
//		int TimeOutCnt;
		volatile int Checked[XDMAPS_CHANNELS_PER_DEV];
		XDmaPs_Config *DmaCfg;
		XDmaPs *DmaInst = &DmaInstance;
		XDmaPs_Cmd DmaCmd;

		memset(&DmaCmd, 0, sizeof(XDmaPs_Cmd));

		DmaCmd.ChanCtrl.SrcBurstSize = 4;
		DmaCmd.ChanCtrl.SrcBurstLen = 4;
		DmaCmd.ChanCtrl.SrcInc = 1;
		DmaCmd.ChanCtrl.DstBurstSize = 4;
		DmaCmd.ChanCtrl.DstBurstLen = 4;
		DmaCmd.ChanCtrl.DstInc = 1;
		DmaCmd.BD.SrcAddr = (u32) Src;
		DmaCmd.BD.DstAddr = (u32) Dst;
		DmaCmd.BD.Length = DMA_LENGTH * sizeof(int);

		DmaCfg = XDmaPs_LookupConfig(DeviceId);
		XDmaPs_CfgInitialize(DmaInst,DmaCfg,DmaCfg->BaseAddress);
		SetupInterrupt(&GicInstance, DmaInst);

		/* Initialize source */
		for (Index = 0; Index < DMA_LENGTH; Index++){
			Src[Index] = DMA_LENGTH - Index;
		}
		/* Clear destination */
		for (Index = 0; Index < DMA_LENGTH; Index++){
			Dst[Index] = 0;
		}
		Checked[Channel] = 0;
		/* Set the Done interrupt handler */
		XDmaPs_SetDoneHandler(DmaInst,Channel,DmaISR,(void *)(Checked + Channel));
		XDmaPs_Start(DmaInst, Channel, &DmaCmd, 0);
		XDmaPs_Print_DmaProg(&DmaCmd);

}

void SetupInterrupt(XScuGic *GicPtr, XDmaPs *DmaPtr)
{
	//int Status;
	XScuGic_Config *GicConfig;


	Xil_ExceptionInit();

	GicConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

	XScuGic_CfgInitialize(GicPtr, GicConfig,GicConfig->CpuBaseAddress);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			     GicPtr);

	XScuGic_Connect(GicPtr,DMA_FAULT_INTR,(Xil_InterruptHandler)XDmaPs_FaultISR,(void *)DmaPtr);


	XScuGic_Connect(GicPtr,DMA_DONE_INTR_0,(Xil_InterruptHandler)XDmaPs_DoneISR_0,(void *)DmaPtr);

	XScuGic_Enable(GicPtr, DMA_DONE_INTR_0);

	Xil_ExceptionEnable();
}

void DmaISR(unsigned int Channel, XDmaPs_Cmd *DmaCmd, void *CallbackRef)
{

	/* done handler */
	volatile int *Checked = (volatile int *)CallbackRef;
	int Index;
	int Status = 1;
	int *Src;
	int *Dst;

	Src = (int *)DmaCmd->BD.SrcAddr;
	Dst = (int *)DmaCmd->BD.DstAddr;

	/* DMA successful */
	/* compare the src and dst buffer */
	for (Index = 0; Index < DMA_LENGTH; Index++) {
		if ((Src[Index] != Dst[Index]) ||
				(Dst[Index] != DMA_LENGTH - Index)) {
			Status = -XST_FAILURE;
		}
	}


	*Checked = Status;
	printf("DMA passed\n\r");
}