#include "HD3CGraphics.h"

uint8_t* getPlaneData(HD3CDriver *d, HD3CGraphics *g) {
	uint8_t curPlane = hd3cDriverGetCurrentPlane(d);

	uint16_t offset = d->planeLedCount * curPlane;
	uint8_t *planeData = g->_data + offset;

	if (curPlane + 1 == d->planeCount) {
		uint8_t *tmp = g->_data;
		g->_data = g->_buffer;
		g->_buffer = tmp;
		g->_drawCube(g, g->_tag);
	}

	return planeData;
}

HD3CGraphics* hd3cGraphicsCreate(HD3CDriver *d) {
	HD3CGraphics *g = calloc(1, sizeof(HD3CGraphics));
	hd3cDriverSetDataProvider(d, g, getPlaneData);
	*((HD3CDriver**)&g->driver) = d;

	g->_data = (uint8_t*)calloc(1, g->driver->ledCount);
	g->_buffer = (uint8_t*)calloc(1, g->driver->ledCount);

	return g;
}

void hd3cGraphicsSetDataProvider(HD3CGraphics *g, void *tag, HD3CGraphicsDrawCubeDelegate drawCube) {
	g->_tag = tag;
	g->_drawCube = drawCube;
}

uint8_t hd3cGraphicsWithinBounds(HD3CGraphics *g, float x, float y, float z) {
	return x >= 0 && y >= 0 && z >= 0 && x < g->driver->planeXLedCount && y < g->driver->planeYLedCount && z < g->driver->planeCount;
}

void hd3cGraphicsSetPixel(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z, uint8_t strength) {
	uint16_t planeOffset = z * g->driver->planeLedCount;
	uint16_t columnOffset = y * g->driver->planeXLedCount;
	*(g->_buffer + planeOffset + columnOffset + x) = strength;
}

void hd3cGraphicsCursorSet(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z) {
	g->_cursor = x;
	g->_cursor = y * g->driver->planeXLedCount;
	g->_cursor = z * g->driver->planeLedCount;
}

void hd3cGraphicsCursorMove(HD3CGraphics *g, uint8_t x, uint8_t y, uint8_t z) {
	g->_cursor += x;
	g->_cursor += y * g->driver->planeXLedCount;
	g->_cursor += z * g->driver->planeLedCount;
}

void hd3cGraphicsCursorXMove(HD3CGraphics *g, uint8_t x) {
	g->_cursor += x;
}

void hd3cGraphicsCursorYMove(HD3CGraphics *g, uint8_t y) {
	g->_cursor += y * g->driver->planeXLedCount;
}

void hd3cGraphicsCursorZMove(HD3CGraphics *g, uint8_t z) {
	g->_cursor += z * g->driver->planeLedCount;
}

void hd3cGraphicsCursorSetPixel(HD3CGraphics *g, uint8_t strength) {
	*(g->_buffer + g->_cursor) = strength;
}

void hd3cGraphicsClear(HD3CGraphics *g) {
	memset(g->_buffer, 0, g->driver->ledCount);
}

void hd3cGraphicsDrawLine(HD3CGraphics *g, uint8_t x0, uint8_t y0, uint8_t z0, uint8_t x1, uint8_t y1, uint8_t z1, uint8_t strength) {
	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int16_t dz = z1 - z0;
	int8_t xDir = dx < 0 ? -1 : 1;
	int8_t yDir = dy < 0 ? -1 : 1;
	int8_t zDir = dz < 0 ? -1 : 1;
	uint8_t xLen = abs(dx);
	uint8_t yLen = abs(dy);
	uint8_t zLen = abs(dz);
	int16_t dx2 = xLen * 2;
	int16_t dy2 = yLen * 2;
	int16_t dz2 = zLen * 2;

	hd3cGraphicsCursorSet(g, x0, y0, z0);
	if (xLen >= yLen && xLen >= zLen) {
		int16_t ey = dy2 - xLen;
		int16_t ez = dz2 - xLen;
		for (int i = 0; i < xLen; i++) {
			hd3cGraphicsCursorSetPixel(g, strength);
			if (ey > 0) {
				hd3cGraphicsCursorYMove(g, yDir);
				ey -= dx2;
			}
			if (ez > 0) {
				hd3cGraphicsCursorZMove(g, zDir);
				ez -= dx2;
			}
			ey += dy2;
			ez += dz2;
			hd3cGraphicsCursorXMove(g, xDir);
		}

	}
	else if (yLen >= xLen && xLen >= zLen) {
		int16_t ex = dx2 - yLen;
		int16_t ez = dz2 - yLen;
		for (int i = 0; i < yLen; i++) {
			hd3cGraphicsCursorSetPixel(g, strength);
			if (ex > 0) {
				hd3cGraphicsCursorXMove(g, xDir);
				ex -= dy2;
			}
			if (ez > 0) {
				hd3cGraphicsCursorZMove(g, zDir);
				ez -= dy2;
			}
			ex += dx2;
			ez += dz2;
			hd3cGraphicsCursorYMove(g, yDir);
		}

	}
	else {
		int16_t ey = dy2 - zLen;
		int16_t ex = dx2 - zLen;
		for (int i = 0; i < zLen; i++) {
			hd3cGraphicsCursorSetPixel(g, strength);
			if (ey > 0) {
				hd3cGraphicsCursorYMove(g, yDir);
				ey -= dz2;
			}
			if (ex > 0) {
				hd3cGraphicsCursorXMove(g, xDir);
				ex -= dz2;
			}
			ey += dy2;
			ex += dx2;
			hd3cGraphicsCursorZMove(g, zDir);
		}
	}
	hd3cGraphicsCursorSetPixel(g, strength);
}

void hd3cGraphicsDrawCube(HD3CGraphics *g, uint8_t cx, uint8_t cy, uint8_t cz, uint8_t xLen, uint8_t yLen, uint8_t zLen, uint8_t strength) {

}