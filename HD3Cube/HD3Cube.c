#include "stm32f1xx.h"

#define NOPDELAY asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")

#define PlaneClockSet GPIOB->BSRR = GPIO_BSRR_BS6 
#define PlaneLatchSet GPIOB->BSRR = GPIO_BSRR_BS7 
#define PlaneEnableSet GPIOB->BSRR = GPIO_BSRR_BS8
#define PlaneDataSet GPIOB->BSRR = GPIO_BSRR_BS9

#define PlaneClockReset GPIOB->BSRR = GPIO_BSRR_BR6
#define PlaneLatchReset GPIOB->BSRR = GPIO_BSRR_BR7
#define PlaneEnableReset GPIOB->BSRR = GPIO_BSRR_BR8 
#define PlaneDataReset GPIOB->BSRR = GPIO_BSRR_BR9 

#define PlaneData(data) GPIOB->BSRR = ((data) ? GPIO_BSRR_BS9 : GPIO_BSRR_BR9)


const uint8_t sideCount = 8;
const uint8_t ledCount = 64;
const uint8_t stepCount = 32;
uint8_t buffer[64];
uint8_t data[64];

uint8_t step = 0;
uint8_t z = 0;
uint16_t t = 0;
uint8_t offset = 0;

int *STCSR = (int *)0xE000E010;
int *STRVR = (int *)0xE000E014;
int *STCVR = (int *)0xE000E018;

volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *SCB_DEMCR = (unsigned int *)0xE000EDFC;

void TIM1_UP_IRQHandler() {
	volatile unsigned int start = *DWT_CYCCNT;
	if (TIM1->SR & TIM_SR_UIF) {
		TIM1->SR &= ~TIM_SR_UIF;
		
		//##  Show Backbufferframe  ##
		if (step == 0) {
			GPIOA->BSRR = 0x200;
			//LEDOE(GPIO_PIN_SET);
		}
		
		GPIOA->BSRR = 0x400;
		NOPDELAY;
		GPIOA->BSRR = 0x4000000;
		//LEDLE(GPIO_PIN_SET);
		//LEDLE(GPIO_PIN_RESET);
		
		if (step == 0) {
			GPIOB->BSRR = 0x80;
			NOPDELAY;
			GPIOB->BSRR = 0x800000;
			//PLANESTR(GPIO_PIN_SET);
			//PLANESTR(GPIO_PIN_RESET);		
		
			GPIOA->BSRR = 0x2000000;
			//LEDOE(GPIO_PIN_RESET);
		}
		//########################
		
		
		t++;
		step = t % stepCount;
		z = (t / stepCount) % 8;
		
		for (size_t i = 0; i < ledCount; i++) {
			GPIOB->BSRR = data[i] > step ? 0x8000 : 0x80000000;
			//LEDDATA(data[colIndex][rowIndex] > i ? GPIO_PIN_SET : GPIO_PIN_RESET);

			GPIOB->BSRR = 0x2000;
			NOPDELAY;
			GPIOB->BSRR = 0x20000000;
			//LEDCLK(GPIO_PIN_SET);
			//LEDCLK(GPIO_PIN_RESET);
		}
			
		if (step == 0) {
			GPIOB->BSRR = z == 0 ? 0x2000000 : 0x200;
			//PLANEIN((t % 16) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);	
		
			GPIOB->BSRR = 0x40;
			NOPDELAY;
			GPIOB->BSRR = 0x400000;
			//PLANECLK(GPIO_PIN_SET);
			//PLANECLK(GPIO_PIN_RESET);
		}
			

	}
	
	volatile unsigned int count = *DWT_CYCCNT - start;
	if (TIM1->SR & TIM_SR_UIF) {
		GPIOC->BSRR = GPIOC->ODR & GPIO_ODR_ODR13 ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
	}
}

