#pragma once

#include <math.h>
#include <string.h>
#include "HD3CDriver.h"


typedef struct _HD3CGraphics HD3CGraphics;

typedef void(*HD3CGraphicsDrawCubeDelegate)(HD3CGraphics *g, void *tag);

struct _HD3CGraphics {
	HD3CDriver *const driver;

	uint8_t *_data;
	uint8_t *_buffer;
	uint16_t _cursor;
	void *_tag;
	HD3CGraphicsDrawCubeDelegate _drawCube;
};

HD3CGraphics* hd3cGraphicsCreate(HD3CDriver *d);

void hd3cGraphicsSetDataProvider(HD3CGraphics *g, void *tag, HD3CGraphicsDrawCubeDelegate drawCube);

uint8_t hd3cGraphicsWithinBounds(HD3CGraphics * g, float x, float y, float z);

void hd3cGraphicsCursorSetPixel(HD3CGraphics *g, uint8_t strength);

void hd3cGraphicsClear(HD3CGraphics *g);

void hd3cGraphicsDrawLine(HD3CGraphics *g, uint8_t x0, uint8_t y0, uint8_t z0, uint8_t x1, uint8_t y1, uint8_t z1, uint8_t strength);

void hd3cGraphicsSetPixel(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z, uint8_t strength);

void hd3cGraphicsCursorSet(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z);

void hd3cGraphicsCursorMove(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z);

void hd3cGraphicsCursorXMove(HD3CGraphics *g, uint8_t x);

void hd3cGraphicsCursorYMove(HD3CGraphics *g, uint8_t y);

void hd3cGraphicsCursorZMove(HD3CGraphics *g, uint8_t z);
