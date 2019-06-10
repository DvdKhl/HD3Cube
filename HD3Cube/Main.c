#include "HD3CRenderer.h"
#include "HD3EffectWaterdrops.h"
#include "SWO.h"

HD3CDriver *hd3cDriver;

void TIM1_UP_IRQHandler() {
	//volatile unsigned int start = *DWT_CYCCNT;

	if (TIM1->SR & TIM_SR_UIF) {
		TIM1->SR &= ~TIM_SR_UIF;
		hd3cDriverPwmTick(hd3cDriver);
	}

	//volatile unsigned int count = *DWT_CYCCNT - start;
	//if (TIM1->SR & TIM_SR_UIF) {
	//	GPIOC->BSRR = GPIO_BSRR_BR13;
	//}
	GPIOC->BSRR = (hd3cDriver->_curPwmStep % 2) == 0 ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
}

void DMA1_Channel5_IRQHandler() {
	SPI2->CR1 &= ~SPI_CR1_SPE; //Disable SPI2
	DMA1->IFCR = DMA_IFCR_CTCIF5;
}



volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *SCB_DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int count;

uint8_t *src;
uint8_t* getPlaneData2(HD3CDriver *d, HD3CGraphics *g) {
	return src;
}






void printLedBuffer(HD3CDriver *d) {
	SWO_PrintString("DST:\n");
	for (size_t p = 0; p < d->ledPwmSteps; p++) {
		for (size_t i = 0; i < d->planeLedCount; i++) {
			SWO_PrintChar(d->_ledBuffer[(p * d->planeLedCount >> 3) + (i >> 3)] & (1 << (i % 8)) ? '1' : '0');
			if ((i % d->planeXLedCount) == d->planeXLedCount - 1) SWO_PrintChar('\n');
		}
		SWO_PrintString("\n");
	}
	SWO_PrintChar('\n');
}

void printPWMData(HD3CDriver *d) {
	uint8_t *data = d->_getPlaneData(d, d->_tag);
	
	uint8_t hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	SWO_PrintString("SRC:\n");
	for (size_t y = 0; y < d->planeYLedCount; y++) {
		for (size_t x = 0; x < d->planeXLedCount; x++) {
			uint8_t val = data[x + y * d->planeXLedCount];
			SWO_PrintChar(hex[(val >> 4) & 0xF]);
			SWO_PrintChar(hex[val & 0xF]);
			SWO_PrintChar(' ');
		}
		SWO_PrintChar('\n');
	}
	SWO_PrintChar('\n');
}



int main() {
	*SCB_DEMCR = *SCB_DEMCR | 0x01000000;
	*DWT_CYCCNT = 0; // reset the counter
	*DWT_CONTROL = *DWT_CONTROL | 1; // enable the counter
	
	hd3cDriver = hd3cDriverCreate();
	hd3cDriverInit(hd3cDriver);
	hd3cDriverSetDataProvider(hd3cDriver, NULL, getPlaneData2);
	
	src = calloc(1, 256);
	
	
	for (size_t i = 0; i < 256; i++) src[i] = (i % 2) != 0 ? 0 : 31;
	//for (size_t i = 0; i < 8 * 32; i++) hd3cDriver->_ledData[i] = (i >> 3) == 31 ? 0b01010101 : 0;
	//for (size_t i = 0; i < 8 * 32; i++) hd3cDriver->_ledBuffer[i] = (i >> 3) == 31 ? 0b01010101 : 0;
	
	//uint64_t *ledData = hd3cDriver->_ledData;
	//uint64_t *ledBuffer = hd3cDriver->_ledBuffer;
	//for (size_t i = 0; i < 32; i++) ledData[i] =   0b1111111111111111111111111111111111111111111111111111111111111111;
	//for (size_t i = 0; i < 32; i++) ledBuffer[i] = 0b1000000010000000010000001000000010000000100000000000010000000000;
	
	

	HD3CGraphics *g = hd3cGraphicsCreate(hd3cDriver);
	HD3CRenderers *r = hd3cRenderersCreate(g, 0.02f, 0.02f, 0.02f);
	HD3EffectWaterdrops *effectWaterdrops = hd3EffectWaterdropsCreate(8, 8, 8, 32);
	hd3cRenderersRegister(r, effectWaterdrops, hd3cEffectWaterdropsRender);

//	printPWMData(hd3cDriver);
//	
//	hd3cDriverPlaneTick(hd3cDriver);
//	printLedBuffer(hd3cDriver);
//	uint8_t *tmp = hd3cDriver->_ledData;
//	hd3cDriver->_ledData = hd3cDriver->_ledBuffer;
//	hd3cDriver->_ledBuffer = tmp;
//	hd3cDriver->_ledBufferState = 0;
//	hd3cDriverPlaneTick(hd3cDriver);
//	printLedBuffer(hd3cDriver);
	
	hd3cDriverStart(hd3cDriver);
	
	GPIOC->BSRR = GPIO_BSRR_BS13;
	while (1) {
		if (hd3cDriverPlaneTick(hd3cDriver)) {
			printPWMData(hd3cDriver);
			printLedBuffer(hd3cDriver);
		}
		//GPIOC->BSRR = GPIO_BSRR_BS13;
		//hd3cDriver->_ledBufferState = 1;
	}
	
	

	

	return 0;
}
