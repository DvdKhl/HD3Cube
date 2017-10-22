#include "HD3CRenderer.h"
#include "HD3EffectWaterdrops.h"

HD3CDriver *hd3cDriver;

void TIM1_UP_IRQHandler() {
	//volatile unsigned int start = *DWT_CYCCNT;

	if (TIM1->SR & TIM_SR_UIF) {
		TIM1->SR &= ~TIM_SR_UIF;
		hd3cDriverPwmTick(hd3cDriver);
	}

	//volatile unsigned int count = *DWT_CYCCNT - start;
	if (TIM1->SR & TIM_SR_UIF) {
		GPIOC->BSRR = GPIO_BSRR_BR13;
	}
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

int main() {
	*SCB_DEMCR = *SCB_DEMCR | 0x01000000;
	*DWT_CYCCNT = 0; // reset the counter
	*DWT_CONTROL = *DWT_CONTROL | 1; // enable the counter
	
	hd3cDriver = hd3cDriverCreate();
	hd3cDriverInit(hd3cDriver);
	hd3cDriverSetDataProvider(hd3cDriver, NULL, getPlaneData2);
	
	src = calloc(1, 256);
	
	
	for (size_t i = 0; i < 256; i++) src[i] = 63;
	for (size_t i = 0; i < 8 * 32; i++) hd3cDriver->_ledData[i] = 0xF0;
	for (size_t i = 0; i < 8 * 32; i++) hd3cDriver->_ledBuffer[i] = 0x0F;
	
	//uint64_t *ledData = hd3cDriver->_ledData;
	//uint64_t *ledBuffer = hd3cDriver->_ledBuffer;
	//for (size_t i = 0; i < 32; i++) ledData[i] =   0b1111111111111111111111111111111111111111111111111111111111111111;
	//for (size_t i = 0; i < 32; i++) ledBuffer[i] = 0b1000000010000000010000001000000010000000100000000000010000000000;
	
	

	//HD3CGraphics *g = hd3cGraphicsCreate(hd3cDriver);
	//HD3CRenderers *r = hd3cRenderersCreate(g, 0.02f, 0.02f, 0.02f);
	//HD3EffectWaterdrops *effectWaterdrops = hd3EffectWaterdropsCreate(16, 16, 16, 512);
	//hd3cRenderersRegister(r, effectWaterdrops, hd3cEffectWaterdropsRender);

	
	GPIOC->BSRR = GPIO_BSRR_BS13;
	while (1) {
		hd3cDriverPlaneTick(hd3cDriver);
		//GPIOC->BSRR = GPIO_BSRR_BS13;
	}
	
	

	

	return 0;
}
