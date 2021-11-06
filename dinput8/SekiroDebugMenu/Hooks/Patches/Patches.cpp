#include "../../SekiroDebugMenu.h"

void DrawMenuHook(uint64_t qUnkClass, SMenuDrawLocation* pLocationData, wchar_t* pwString) {

	CGraphics* Graphics = GetGraphics();

	int iArraySize = Graphics->GetDebugPrintArraySize();
	SDebugPrintStruct* pStruct = Graphics->GetDebugPrintArrayStart();

	Graphics->EnterCS();

	for (int i = 0; i < iArraySize; i++, pStruct++) {
		if (pStruct->bIsActive) continue;
		memcpy(pStruct->wText, pwString, 512);
		pStruct->fX = pLocationData->f1;
		pStruct->fY = pLocationData->f2;
		pStruct->bIsActive = true;
		break;
	};

	Graphics->LeaveCS();
	return;
};