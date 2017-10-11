#pragma once

#include "HD3CRenderer.h"

typedef struct _HD3EffectWaterdrops HD3EffectWaterdrops;

HD3EffectWaterdrops* hd3EffectWaterdropsCreate(uint8_t areaX, uint8_t areaY, uint8_t areaZ, uint16_t dropCount);

void hd3cEffectWaterdropsRender(HD3CRenderers *r, HD3EffectWaterdrops *e);
