#include "HD3CDriver.h"

#define DMA

#define NOPDELAY asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")

#define PlaneClockSet()    GPIOB->BSRR = GPIO_BSRR_BS6 
#define PlaneLatchSet()    GPIOB->BSRR = GPIO_BSRR_BS7 
#define PlaneEnableSet()   GPIOB->BSRR = GPIO_BSRR_BS8
#define PlaneDataSet()     GPIOB->BSRR = GPIO_BSRR_BS9
#define PlaneClockReset()  GPIOB->BSRR = GPIO_BSRR_BR6
#define PlaneLatchReset()  GPIOB->BSRR = GPIO_BSRR_BR7
#define PlaneEnableReset() GPIOB->BSRR = GPIO_BSRR_BR8 
#define PlaneDataReset()   GPIOB->BSRR = GPIO_BSRR_BR9 

#define PlaneData(data) GPIOB->BSRR = (data) ? GPIO_BSRR_BS9 : GPIO_BSRR_BR9
#define PlaneStep()  PlaneClockSet(); NOPDELAY; PlaneClockReset()
#define PlaneLatch() PlaneLatchSet(); NOPDELAY; PlaneLatchReset()


#define LedClockSet()    GPIOB->BSRR = GPIO_BSRR_BS13
#define LedClockReset()  GPIOB->BSRR = GPIO_BSRR_BR13
#define LedLatchSet()    GPIOA->BSRR = GPIO_BSRR_BS10
#define LedLatchReset()  GPIOA->BSRR = GPIO_BSRR_BR10
#define LedEnableSet()   GPIOA->BSRR = GPIO_BSRR_BS9
#define LedEnableReset() GPIOA->BSRR = GPIO_BSRR_BR9

#define LedStep()  LedClockSet(); NOPDELAY; LedClockReset()
#define LedLatch() LedLatchSet(); NOPDELAY; LedLatchReset()
#define LedData(data) GPIOB->BSRR = (data) ? GPIO_BSRR_BS15 : GPIO_BSRR_BR15;


HD3CDriver* hd3cDriverCreate() {
	HD3CDriver *d = (HD3CDriver*)calloc(1, sizeof(HD3CDriver));
	//*((uint8_t*)&d->planeCount) = 16;
	//*((uint8_t*)&d->planeXLedCount) = 16;
	//*((uint8_t*)&d->planeYLedCount) = 16;
	//*((uint16_t*)&d->planeLedCount) = d->planeXLedCount * d->planeYLedCount;
	//*((uint16_t*)&d->ledCount) = (uint16_t)d->planeLedCount * (uint16_t)d->planeCount;
	//*((uint8_t*)&d->cubeFrequency) = 100;
	//*((uint8_t*)&d->ledPwmSteps) = 32;
	
	d->planeCount = 8;
	d->planeXLedCount = 8;
	d->planeYLedCount = 8;
	d->planeLedCount = d->planeXLedCount * d->planeYLedCount;
	d->ledCount = d->planeLedCount * d->planeCount;
	d->cubeFrequency = 100;
	d->ledPwmSteps = 32;
	
	d->_ledData = calloc(1, d->planeLedCount * d->ledPwmSteps / 8);
	d->_ledBuffer = calloc(1, d->planeLedCount * d->ledPwmSteps / 8);
	
	return d;
}
void hd3cDriverPwmTick(HD3CDriver *d);

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
void hd3cPlaneInit(HD3CDriver *d) {
	PlaneEnableSet();
	PlaneLatchReset();
	PlaneClockReset();
	PlaneDataSet();
	for (size_t i = 0; i < d->planeCount; i++) {
		PlaneStep();
	}
	PlaneLatch();
}
void hd3cDriverLedsInit(HD3CDriver *d) {
	LedEnableReset();
	LedLatchReset();
	LedClockReset();
	LedData(0);
	for (size_t i = 0; i < d->planeLedCount; i++) {
		LedStep();
	}
	LedLatch();
}
void hd3cPinInit() {
	//Enable GPIO Ports A, B and C
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;

	//Enable SPI2
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN | RCC_APB2ENR_AFIOEN;

	//Enable DMA1
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
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

	//PC13: CNF(Out PP) Mode(High)    - MC Led
	GPIOC->CRH = (GPIOC->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1;
	
	//###########
	//##  SPI  ##
	//###########
//	//PB13: CNF(Out OD) Mode(High)     - LedDriver Clock
//	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_CNF13_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1;
//	//PB15: CNF(Out OD) Mode(High)     - LedDriver Data To Driver
//	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15)) | GPIO_CRH_CNF15_0 | GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1;

	//PB13: CNF(AF OD) Mode(High)     - LedDriver Clock
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_CNF13_0 | GPIO_CRH_CNF13_1 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1;
	//PB15: CNF(AF OD) Mode(High)     - LedDriver Data To Driver
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15)) | GPIO_CRH_CNF15_0 | GPIO_CRH_CNF15_1 | GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1;
	
	
	SPI2->CR1 &= ~SPI_CR1_SPE; //Disable SPI2
	SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI; //Master mode
	SPI2->CR1 |= SPI_CR1_CPHA; //SPI_PHASE_2EDGE
	SPI2->CR1 |= SPI_CR1_CPOL; //SPI_POLARITY_HIGH
	SPI2->CR1 |= SPI_CR1_DFF; //SPI_DATASIZE_16BIT
	SPI2->CR1 |= SPI_CR1_SSM; //SPI_NSS_SOFT
	//SPI2->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0; //SPI_BAUDRATEPRESCALER_256
	//SPI2->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1; //SPI_BAUDRATEPRESCALER_128
	//SPI2->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0; //SPI_BAUDRATEPRESCALER_64
	//SPI2->CR1 |= SPI_CR1_BR_2; //SPI_BAUDRATEPRESCALER_32
	//SPI2->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0; //SPI_BAUDRATEPRESCALER_16
	SPI2->CR1 |= SPI_CR1_BR_1; //SPI_BAUDRATEPRESCALER_8
	//SPI2->CR1 |= SPI_CR1_BR_0; //SPI_BAUDRATEPRESCALER_4
	SPI2->SR |= SPI_SR_TXE; //Send Buffer ist empty
	
	
#ifdef DMA
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	DMA1_Channel5->CCR = 
		//DMA_CCR_MEM2MEM |
		DMA_CCR_PL_0 | DMA_CCR_PL_1 | //Priority Very High
		//DMA_CCR_MSIZE_0 | //16bit
		//DMA_CCR_PSIZE_0 | //16bit
		DMA_CCR_MINC | //Increment Memory
		//DMA_CCR_PINC |
		//DMA_CCR_CIRC |
		DMA_CCR_DIR | //Read from Memory
		DMA_CCR_TCIE | //Interrupt Complete enable
		0;
#endif
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
#ifdef DMA
	TIM1->PSC = 64;
	TIM1->ARR = 40000;
#else
	TIM1->PSC = 128;
	TIM1->ARR = 60000;
#endif
	TIM1->CR1 = TIM_CR1_CEN; //Enable Counter

}
int hd3cDriverInit(HD3CDriver *d) {
	hd3cSysClockInit();
	hd3cPinInit();
	hd3cPlaneInit(d);
	hd3cDriverLedsInit(d);
	hd3cTimerInit();

	//PlaneData(0);
	//PlaneStep();
	
	
	return 0;
}

void hd3cDriverStart(HD3CDriver *d) {
	TIM1->DIER = TIM_DIER_UIE; //Enable Update Interrupt
}

void hd3cDriverLedDmaStart(HD3CDriver *d) {
#ifdef DMA
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	DMA1_Channel5->CNDTR = d->planeLedCount >> 3;
	DMA1_Channel5->CPAR = (uint32_t)&SPI2->DR;
	DMA1_Channel5->CMAR = (uint32_t)(d->_ledData + d->_curPwmStep * (d->planeLedCount >> 3));
	DMA1_Channel5->CCR |= DMA_CCR_EN;
	
	SPI2->CR1 |= SPI_CR1_SPE; //Enable SPI2
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
#else
	SPI2->CR1 |= SPI_CR1_SPE; //Enable SPI2

	uint8_t *_ledData = d->_ledData + d->_curPwmStep * (d->planeLedCount >> 3);
	for (size_t i = 0; i < 8; i++) {
		//SPI2->DR = 0b0101010101010101;
		SPI2->DR = _ledData[i];
		while (!(SPI2->SR & SPI_SR_TXE));
	}
	while (SPI2->SR & SPI_SR_BSY);

	SPI2->CR1 &= ~SPI_CR1_SPE; //Disable SPI2
#endif
}

uint8_t hd3cDriverPlaneTick(HD3CDriver *d) {
	if (d->_ledBufferState) return 0;

	
	d->_ledPwmData = d->_getPlaneData(d, d->_tag);

	uint32_t *ledBuffer = d->_ledBuffer;
	size_t ledBufferIntCount = (d->planeLedCount * d->ledPwmSteps) >> 5;
	for (size_t i = 0; i < ledBufferIntCount; i++) {
		ledBuffer[i] = 0;
	}	
	//memset(d->_ledBuffer, 0, (d->planeLedCount * d->ledPwmSteps) >> 3);
	
	size_t planeByteCount = d->planeLedCount >> 3;
	for (size_t i = 0; i < d->planeLedCount; i += 8) {
		size_t j = i >> 3;
		d->_ledBuffer[(d->_ledPwmData[i + 0] * planeByteCount) + j] += 0x01;
		d->_ledBuffer[(d->_ledPwmData[i + 1] * planeByteCount) + j] += 0x02;
		d->_ledBuffer[(d->_ledPwmData[i + 2] * planeByteCount) + j] += 0x04;
		d->_ledBuffer[(d->_ledPwmData[i + 3] * planeByteCount) + j] += 0x08;
		d->_ledBuffer[(d->_ledPwmData[i + 4] * planeByteCount) + j] += 0x10;
		d->_ledBuffer[(d->_ledPwmData[i + 5] * planeByteCount) + j] += 0x20;
		d->_ledBuffer[(d->_ledPwmData[i + 6] * planeByteCount) + j] += 0x40;
		d->_ledBuffer[(d->_ledPwmData[i + 7] * planeByteCount) + j] += 0x80;
	}


	size_t planeLedIntCount = d->planeLedCount >> 5;
	for (int j = ledBufferIntCount - planeLedIntCount; j >= planeLedIntCount; j -= planeLedIntCount) {
//		for (size_t i = 0; i < planeLedIntCount; i += 8) {
//			ledBuffer[j + i + 0] += ledBuffer[j - planeLedIntCount + i + 0];
//			ledBuffer[j + i + 1] += ledBuffer[j - planeLedIntCount + i + 1];
//			ledBuffer[j + i + 2] += ledBuffer[j - planeLedIntCount + i + 2];
//			ledBuffer[j + i + 3] += ledBuffer[j - planeLedIntCount + i + 3];
//			ledBuffer[j + i + 4] += ledBuffer[j - planeLedIntCount + i + 4];
//			ledBuffer[j + i + 5] += ledBuffer[j - planeLedIntCount + i + 5];
//			ledBuffer[j + i + 6] += ledBuffer[j - planeLedIntCount + i + 6];
//			ledBuffer[j + i + 7] += ledBuffer[j - planeLedIntCount + i + 7];
//		}
		
		
		for (int i = planeLedIntCount - 2; i >= 0; i -= 2) {
			ledBuffer[j - planeLedIntCount + i + 0] += ledBuffer[j + i + 0];
			ledBuffer[j - planeLedIntCount + i + 1] += ledBuffer[j + i + 1];
		}
		
	}
	asm volatile("" : : : "memory");
	d->_ledBufferState = 1;
	return 1;
}

void hd3cDriverPwmTickAsm(HD3CDriver *d) {
	//asm(
	//	""
	//)
}

void hd3cDriverPwmTick(HD3CDriver *d) {
	//LedEnableSet();
	//PlaneEnableReset();
	if (d->_curPwmStep == 0) {
		PlaneLatch();
	}
	LedLatch();
	//PlaneEnableSet();
	//LedEnableReset();

	d->_curPwmStep = (d->_curPwmStep + 1) % d->ledPwmSteps;
	if (d->_curPwmStep == 0) {
		if (!d->_ledBufferState) GPIOC->BSRR = GPIO_BSRR_BR13;
		
		uint8_t *tmp = d->_ledData;
		d->_ledData = d->_ledBuffer;
		d->_ledBuffer = tmp;

		if (d->_curPlane < 2) PlaneData(d->_curPlane != 0);
		d->_curPlane = (d->_curPlane + 1) % d->planeCount;
		PlaneStep();
		d->_ledBufferState = 0;
	} 
	if(d->_curPwmStep < 32) 	hd3cDriverLedDmaStart(d);
}