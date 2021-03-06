#ifndef PXA260_DMA_H
#define PXA260_DMA_H

#include "pxa260_CPU.h"
#include "pxa260_IC.h"

/*
	PXA260 OS DMA controller

*/

#define PXA260_DMA_BASE		0x40000000UL
#define PXA260_DMA_SIZE		0x00001000UL

typedef struct{
	
	UInt32 DAR;	//descriptor address register
	UInt32 SAR;	//source address register
	UInt32 TAR;	//target address register
	UInt32 CR;	//command register
	UInt32 CSR;	//control and status register
	
}Pxa260dmaChannel;

typedef struct{

	Pxa260ic* ic;
	
	UInt16 DINT;
	Pxa260dmaChannel channels[16];
	UInt8 CMR[40];			//channel map registers	[  we store lower 8 bits only :-)  ]
	
}Pxa260dma;

void pxa260dmaInit(Pxa260dma* gpio, Pxa260ic* ic);

#endif

