#include "HD3EffectWaterdrops.h"


typedef struct {
	uint8_t x;
	uint8_t y;
} HD3EffectWaterdropsPos;

typedef struct {
	float height;
	uint32_t createdOn;
	HD3EffectWaterdropsPos position;
} HD3EffectWaterdropsItem;

struct _HD3EffectWaterdrops {
	uint32_t elapsed;
	float acceleration;
	uint8_t areaX;
	uint8_t areaY;
	uint8_t areaZ;

	uint8_t *dropReady;
	HD3EffectWaterdropsPos *dropPermutation;
	HD3EffectWaterdropsItem *drops;
	uint16_t dropCount;
};


void hd3cEffectWaterdropsRandomize(HD3EffectWaterdrops *e) {
	uint16_t dropSlotCount = e->areaX * e->areaY;
	for (size_t i = 0; i <dropSlotCount; i++) {
		size_t j = rand() % dropSlotCount;
		HD3EffectWaterdropsPos tmp = *(e->dropPermutation + i);
		*(e->dropPermutation + i) = *(e->dropPermutation + j);
		*(e->dropPermutation + j) = tmp;
	}
}

HD3EffectWaterdrops* hd3EffectWaterdropsCreate(uint8_t areaX, uint8_t areaY, uint8_t areaZ, uint16_t dropCount) {
	HD3EffectWaterdrops *e = calloc(1, sizeof(HD3EffectWaterdrops));
	e->acceleration = 9.81f;
	e->areaX = areaX;
	e->areaY = areaY;
	e->areaZ = areaZ;

	e->dropCount = dropCount;
	e->drops = calloc(dropCount, sizeof(HD3EffectWaterdropsItem));
	e->dropPermutation = calloc(areaX * areaY, sizeof(HD3EffectWaterdropsPos));
	e->dropReady = calloc((areaX * areaY + 7) / 8, 1);

	for (uint8_t y = 0; y < areaY; y++) {
		for (uint8_t x = 0; x < areaX; x++) {
			HD3EffectWaterdropsPos *pos = (e->dropPermutation + x + y * areaX);
			pos->x = x;
			pos->y = y;
		}
	}
	hd3cEffectWaterdropsRandomize(e);

	return e;
}

void hd3EffectWaterdropsTick(HD3EffectWaterdrops *e, uint8_t timeDelta) {
	e->elapsed += timeDelta;
}

void hd3cEffectWaterdropsRender(HD3CRenderers *r, HD3EffectWaterdrops *e) {
	for (size_t i = 0; i < e->dropCount; i++) {
		HD3EffectWaterdropsItem *drop = e->drops + i;

		int16_t x = drop->position.x - (uint16_t)r->offsetX;
		int16_t y = drop->position.y - (uint16_t)r->offsetY;
		float z = drop->height - r->offsetY;

		if (!hd3cGraphicsWithinBounds(r->graphics, x, y, z)) continue;

		hd3cGraphicsSetPixel(r->graphics, (uint8_t)x, (uint8_t)y, (uint8_t)z, 9);
		hd3cGraphicsSetPixel(r->graphics, (uint8_t)x, (uint8_t)y, (uint8_t)z + 1, 9);
	}
}
