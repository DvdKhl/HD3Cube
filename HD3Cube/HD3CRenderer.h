#pragma once

#include "HD3CGraphics.h"

typedef struct _HD3CRenderer HD3CRenderer;
typedef struct _HD3CRenderers HD3CRenderers;
typedef void(*HD3CRendererDelegate)(HD3CRenderers *r, void *tag);

struct _HD3CRenderer {
	void* tag;
	HD3CRendererDelegate render;
	HD3CRenderer *next;
};

struct _HD3CRenderers {
	const float ledXGap;
	const float ledYGap;
	const float ledZGap;
	float offsetX;
	float offsetY;
	float offsetZ;
	HD3CGraphics *const graphics;
	HD3CRenderer *root;
	void(*render)(HD3CRenderers *r);
};



HD3CRenderers *hd3cRenderersCreate(HD3CGraphics *g, float ledXGap, float ledYGap, float ledZGap);

void hd3cRendererDrawAll(HD3CRenderers *r);

HD3CRenderer* hd3cRenderersRegister(HD3CRenderers *r, void *tag, HD3CRendererDelegate renderDlg);
