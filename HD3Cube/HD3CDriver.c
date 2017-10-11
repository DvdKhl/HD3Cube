#include "HD3CDriver.h"

#define NOPDELAY asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")

#define PlaneClockSet() GPIOB->BSRR = GPIO_BSRR_BS6 
#define PlaneLatchSet() GPIOB->BSRR = GPIO_BSRR_BS7 
#define PlaneEnableSet() GPIOB->BSRR = GPIO_BSRR_BS8
#define PlaneDataSet() GPIOB->BSRR = GPIO_BSRR_BS9
#define PlaneClockReset() GPIOB->BSRR = GPIO_BSRR_BR6
#define PlaneLatchReset() GPIOB->BSRR = GPIO_BSRR_BR7
#define PlaneEnableReset() GPIOB->BSRR = GPIO_BSRR_BR8 
#define PlaneDataReset() GPIOB->BSRR = GPIO_BSRR_BR9 
#define PlaneData(data) GPIOB->BSRR = (data) ? GPIO_BSRR_BS9 : GPIO_BSRR_BR9
#define PlaneStep() PlaneClockSet(); NOPDELAY; PlaneClockReset()
#define PlaneLatch() PlaneLatchSet(); NOPDELAY; PlaneLatchReset()

#define LedClockSet() GPIOB->BSRR = 0x2000
#define LedClockReset() GPIOB->BSRR = 0x20000000
#define LedStep() LedClockSet(); NOPDELAY; LedClockReset()
#define LedLatchSet() GPIOB->BSRR = 0x400
#define LedLatchReset() GPIOB->BSRR = 0x4000000
#define LedLatch() LedClockSet(); NOPDELAY; LedClockReset()
#define LedData(data) GPIOB->BSRR = (data) ? 0x8000 : 0x80000000;

HD3CDriver* hd3cDriverCreate() {
	HD3CDriver *d = (HD3CDriver*)calloc(1, sizeof(HD3CDriver));
	*((uint8_t*)&d->planeCount) = 8;
	*((uint8_t*)&d->planeXLedCount) = 8;
	*((uint8_t*)&d->planeYLedCount) = 8;
	*((uint16_t*)&d->planeLedCount) = d->planeXLedCount * d->planeYLedCount;
	*((uint16_t*)&d->ledCount) = d->planeLedCount * d->planeCount;
	*((uint8_t*)&d->cubeFrequency) = 100;
	*((uint8_t*)&d->ledPwmSteps) = 8;
	
	d->_ledData = calloc(1, d->ledCount * d->ledPwmSteps / 8);
	d->_ledBuffer = calloc(1, d->ledCount * d->ledPwmSteps / 8);
	
	return d;
}
void hd3cDriverTick(HD3CDriver *d);

void hd3cDriverSetDataProvider(HD3CDriver *d, void *tag, HD3CDriverDataProviderDelegate getPlaneData) {
	d->_tag = tag;
	d->_getPlaneData = getPlaneData;
}

uint8_t hd3cDriverGetCurrentPlane(HD3CDriver *d) {
	return d->_curPlane;
}


//## Private ##

void hd3cSysClockInit() {
	RCC->CR |= RCC_CR_HSEON; //Enable HSE
	while (!(RCC->CR & RCC_CR_HSERDY));

	RCC->CFGR = RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9; //Config PLL
	RCC->CR |= RCC_CR_PLLON; //Enable PLL
	while (!(RCC->CR & RCC_CR_PLLRDY));

	FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_2; //Flash Access Latency

	RCC->CFGR |= (RCC->CFGR & ~RCC_CFGR_HPRE) | RCC_CFGR_HPRE_DIV1; //AHB Clock Divider
	RCC->CFGR |= (RCC->CFGR & ~RCC_CFGR_PPRE1) | RCC_CFGR_PPRE1_DIV2; //APB1 Clock Divider
	RCC->CFGR |= (RCC->CFGR & ~RCC_CFGR_PPRE2) | RCC_CFGR_PPRE2_DIV1; //APB2 Clock Divider

	RCC->CFGR |= RCC_CFGR_SW_PLL; //PLL as System Clock Source
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL));
}
void hd3cPlaneInit() {
	PlaneEnableSet();
	PlaneLatchReset();
	PlaneClockReset();
	PlaneDataSet();
	for (size_t i = 0; i < 16; i++) {
		PlaneStep();
	}

	PlaneDataReset();
	for (size_t i = 0; i < 16; i++) {
		PlaneData((i % 2) != 0);
		PlaneStep();
	}
	PlaneLatch();
}
void hd3cPinInit() {
	//Enable GPIO Ports A, B and C
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;

	//PA08: CNF(In NoPull) Mode(High) - LedDriver Data To MC
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8)) | GPIO_CRH_CNF8_0;
	//PA09: CNF(Out OD) Mode(High)    - LedDriver Output Enable
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9)) | GPIO_CRH_CNF9_0 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1;
	//PA10: CNF(Out OD) Mode(High)    - LedDriver Latch Enable
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10)) | GPIO_CRH_CNF10_0 | GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1;

	//PB04: CNF(In NoPull) Mode(High) - PlaneDriver Data To MC
	GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4)) | GPIO_CRL_CNF4_0;
	//PB06: CNF(Out OD) Mode(High)    - PlaneDriver Clock
	GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6)) | GPIO_CRL_CNF6_0 | GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1;
	//PB07: CNF(Out OD) Mode(High)    - PlaneDriver Latch
	GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_CNF7 | GPIO_CRL_MODE7)) | GPIO_CRL_CNF7_0 | GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1;
	//PB08: CNF(Out OD) Mode(High)    - PlaneDriver Output Enable
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8)) | GPIO_CRH_CNF8_0 | GPIO_CRH_MODE8_0 | GPIO_CRH_MODE8_1;
	//PB09: CNF(Out OD) Mode(High)    - PlaneDriver Data To Driver
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9)) | GPIO_CRH_CNF9_0 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1;
	//PB13: CNF(Out OD) Mode(High)    - LedDriver Clock
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_CNF13_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1;
	//PB15: CNF(Out OD) Mode(High)    - LedDriver Data To Driver
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15)) | GPIO_CRH_CNF15_0 | GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1;

	//PC13: CNF(Out PP) Mode(High)    - MC Led
	GPIOC->CRH = (GPIOC->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1;
}
void hd3cTimerInit() {
	//	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	//	NVIC_EnableIRQ(TIM2_IRQn);
	//	TIM2->SR = 0;
	//	TIM2->PSC = 36 * 1024 - 1;
	//	TIM2->ARR = 10 * 1024;
	//	TIM2->CR1 = TIM_CR1_CEN; //Enable Counter
	//	TIM2->DIER = TIM_DIER_UIE; //Enable Update Interrupt
	//	

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	TIM1->SR = 0;
	TIM1->PSC = 0;
	//TIM1->ARR = 704 * 2;
	TIM1->ARR = 4500;
	TIM1->CR1 = TIM_CR1_CEN; //Enable Counter
	TIM1->DIER = TIM_DIER_UIE; //Enable Update Interrupt

}
int hd3cDriverInit(HD3CDriver *d) {
	GPIOC->BSRR = GPIO_BSRR_BS13;

	hd3cSysClockInit();
	hd3cPinInit();
	hd3cPlaneInit();

	hd3cTimerInit();

	/*
	GPIO_InitStructure.GPIO_Pin = WIZ_SCLK | WIZ_MISO | WIZ_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NONE;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	*/
	/*
	GPIO_InitStructure.GPIO_Pin = WIZ_SCS | WIZ_nRST;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, WIZ_SCS);
	GPIO_SetBits(GPIOA, WIZ_nRST);

	// Connect SPI pins to AF_SPI1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); //SCLK
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); //MOSI
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1); //MISO
	*/
	/*
	SPI_InitTypeDef SPI_InitStructure;
	//SPI Config
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	*/
	/* Deinitialize DMA Streams
	DMA_DeInit(DMA2_Stream3); //SPI1_TX_DMA_STREAM
	DMA_DeInit(DMA2_Stream2); //SPI1_RX_DMA_STREAM
	*/
	/*
	DMA_InitStructure.DMA_BufferSize = d->planeXLedCount * d->planeYLedCount;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	//DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DR));
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	//Configure Tx DMA
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pTmpBuf1;
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	//Configure Rx DMA
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pTmpBuf1;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	*/


	return 0;
}

void hd3cDriverTick(HD3CDriver *d) {
	LedLatch();
	if(d->_curPwmStep == 0) PlaneLatch();


	//TODO: SPI DMA
	for (size_t i = 0; i < d->planeLedCount; i++) {
		LedData(d->_ledData[i] > d->_curPwmStep);
		LedStep();
	}

	d->_curPwmStep = (d->_curPwmStep + 1) % d->ledPwmSteps;
	if (d->_curPwmStep == 0) {
		if(d->_curPlane < 2) PlaneData(d->_curPlane == 0);
		PlaneStep();

		d->_ledPwmData = d->_getPlaneData(d, d->_tag);
		d->_curPlane = (d->_curPlane + 1) % d->planeCount;
	}

	int byteCount = d->planeXLedCount * d->planeYLedCount / 8;
	for (int i = 0; i < byteCount; i++) {
		char data = 0;
		for (int j = 0; j < 8; j++) {
			data |= (d->_ledPwmData[i] > d->_curPwmStep ? 1 : 0) << j;
		}
		d->_ledData[i] = data;
	}
}


