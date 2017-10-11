#include "HD3CRenderer.h"
#include "HD3EffectWaterdrops.h"

HD3CDriver *hd3cDriver;

void TIM1_UP_IRQHandler() {
	//volatile unsigned int start = *DWT_CYCCNT;

	if (TIM1->SR & TIM_SR_UIF) {
		TIM1->SR &= ~TIM_SR_UIF;
		hd3cDriverTick(hd3cDriver);
	}

	//volatile unsigned int count = *DWT_CYCCNT - start;
	if (TIM1->SR & TIM_SR_UIF) {
		GPIOC->BSRR = GPIOC->ODR & GPIO_ODR_ODR13 ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
	}
}

int ledCount;
uint8_t *src;
uint8_t *dst;
int *STCSR = (int *)0xE000E010;
int *STRVR = (int *)0xE000E014;
int *STCVR = (int *)0xE000E018;

volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *SCB_DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int count;

void loop(uint8_t pwmStep) {
        //Latch LedController Data to Output
	volatile unsigned int start = *DWT_CYCCNT;
	for (int i = 0; i < ledCount; i += 8) {
		dst[i >> 3] = 
		    (src[i | 0] > pwmStep ? 0x01 : 0) |
		    (src[i | 1] > pwmStep ? 0x02 : 0) |
		    (src[i | 2] > pwmStep ? 0x04 : 0) |
		    (src[i | 3] > pwmStep ? 0x08 : 0) |
		    (src[i | 4] > pwmStep ? 0x10 : 0) |
		    (src[i | 5] > pwmStep ? 0x20 : 0) |
		    (src[i | 6] > pwmStep ? 0x40 : 0) |
		    (src[i | 7] > pwmStep ? 0x80 : 0);
	}
	count = *DWT_CYCCNT - start;
//Transfer dst via DMA-SPI to LedControllers
}

uint8_t *lookup;
void loop2(uint8_t pwmStep) {
	volatile unsigned int start = *DWT_CYCCNT;
    //Latch LedController Data to Output
	uint8_t *lookupShift = lookup + (pwmStep << 6);
	for (int i = 0; i < ledCount; i += 8) {
		dst[i >> 3] = 
		    (lookupShift[src[i | 0]] << 0) |
		    (lookupShift[src[i | 1]] << 1) |
		    (lookupShift[src[i | 2]] << 2) |
		    (lookupShift[src[i | 3]] << 3) |
		    (lookupShift[src[i | 4]] << 4) |
		    (lookupShift[src[i | 5]] << 5) |
		    (lookupShift[src[i | 6]] << 6) |
		    (lookupShift[src[i | 7]] << 7);
	}
	count = *DWT_CYCCNT - start;
	//Transfer dst via DMA-SPI to LedControllers
}

int main() {
	
	*SCB_DEMCR = *SCB_DEMCR | 0x01000000;
	*DWT_CYCCNT = 0; // reset the counter
	*DWT_CONTROL = *DWT_CONTROL | 1; // enable the counter
	
	lookup = calloc(1, 64 * 64);
	for (size_t i = 0; i < 64; i++) {
		for (size_t j = 0; j < 64; j++) {
			lookup[i | (j << 7)] = i > j ? 1 : 0;
		}
	}
	
	ledCount = 256;
	src = calloc(1, ledCount);
	dst = calloc(1, ledCount / 8);

	for (size_t i = 0; i < ledCount; i++) src[i] = i % 64;
	for (uint8_t pwmStep = 0; pwmStep < 32; pwmStep++) {
		loop2(pwmStep);
	}
	

	

	
	hd3cDriver = hd3cDriverCreate();
	hd3cDriverInit(hd3cDriver);

	HD3CGraphics *g = hd3cGraphicsCreate(hd3cDriver);

	HD3CRenderers *r = hd3cRenderersCreate(g, 0.02f, 0.02f, 0.02f);

	HD3EffectWaterdrops *effectWaterdrops = hd3EffectWaterdropsCreate(16, 16, 16, 512);
	hd3cRenderersRegister(r, effectWaterdrops, hd3cEffectWaterdropsRender);

	return 0;
}
