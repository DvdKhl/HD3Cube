#include "HD3CRenderer.h"


void drawCube(HD3CGraphics *g, HD3CRenderers *r) {
	hd3cGraphicsClear(g);
	r->render(r);
}


HD3CRenderers* hd3cRenderersCreate(HD3CGraphics *g, float ledXGap, float ledYGap, float ledZGap) {
	HD3CRenderers *r = calloc(1, sizeof(HD3CRenderers));
	hd3cGraphicsSetDataProvider(g, r, drawCube);
	*((HD3CGraphics **)&r->graphics) = g;
	r->render = hd3cRendererDrawAll;

	return r;
}

void hd3cRendererDrawAll(HD3CRenderers *r) {
	HD3CRenderer *item = r->root;
	while (item != NULL) {
		item->render(r, item->tag);
		item = item->next;
	}
}

HD3CRenderer* hd3cRenderersRegister(HD3CRenderers *r, void* tag, HD3CRendererDelegate renderDlg) {
	HD3CRenderer *item = calloc(1, sizeof(HD3CRenderer));
	item->tag = tag;
	item->render = renderDlg;

	HD3CRenderer **curItem = &r->root;
	while (*curItem != NULL) {
		curItem = &(*curItem)->next;
	}
	*curItem = item;
	return item;
}
