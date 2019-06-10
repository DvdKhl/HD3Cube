#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f1xx.h"


//int *STCSR = (int *)0xE000E010;
//int *STRVR = (int *)0xE000E014;
//int *STCVR = (int *)0xE000E018;
//
//volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
//volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
//volatile unsigned int *SCB_DEMCR = (unsigned int *)0xE000EDFC;


typedef struct _HD3CDriver HD3CDriver;

typedef uint8_t* (*HD3CDriverDataProviderDelegate)(HD3CDriver *d, void *tag);

struct _HD3CDriver {
	uint8_t planeCount;
	uint8_t planeXLedCount;
	uint8_t planeYLedCount;
	uint16_t planeLedCount;
	uint16_t ledCount;
	uint8_t cubeFrequency;
	uint8_t ledPwmSteps;

	void* _tag;
	HD3CDriverDataProviderDelegate _getPlaneData;
	uint8_t _curPlane;
	uint8_t _curPwmStep;
	uint8_t *_ledPwmData;
	uint8_t *_ledData;
	uint8_t *_ledBuffer;
	volatile uint8_t _ledBufferState;
};

HD3CDriver *hd3cDriverCreate();
int hd3cDriverInit(HD3CDriver *d);
void hd3cDriverPwmTick(HD3CDriver *d);
uint8_t hd3cDriverPlaneTick(HD3CDriver *d);

void hd3cDriverSetDataProvider(HD3CDriver *d, void *tag, HD3CDriverDataProviderDelegate getPlaneData);
uint8_t hd3cDriverGetCurrentPlane(HD3CDriver *d);
void hd3cDriverStart(HD3CDriver *d);