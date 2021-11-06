#include "../SekiroDebugMenu.h"
#include "HookSites.h"

bool CHooks::Patch() {

	if (bIsMHActive != MH_OK) {
	
		return false;
	};

	bool bReturn = true;
	uint8_t pMovAl01Bytes[] = { 0xB0, 0x01 };
	uint8_t pXorAlBytes[] = { 0x30, 0xC0 };
	uint8_t pCallBytes[] = { 0xE8 };
	uint8_t pRetBytes[] = { 0xC3 };

	while (!qActivateDebugMenu) {
		qActivateDebugMenu = GetSignature()->GetSignature(&sActivateDebugMenu);
		Sleep(25);
	};

	qActivateDebugMenu += 9;
	bReturn &= Tweak((void*)qActivateDebugMenu, pMovAl01Bytes, sizeof(pMovAl01Bytes));

	while (!GetHookSites())
		Sleep(100);

	bReturn &= Hook((void*)qMenuDrawHook, DrawMenuHook, 0, 0) & Tweak((void*)qMenuDrawHook, pCallBytes, sizeof(pCallBytes));
	bReturn &= Tweak((void*)qMissingParamHook1, pRetBytes, sizeof(pRetBytes));
	bReturn &= Tweak((void*)qMissingParamHook2, pRetBytes, sizeof(pRetBytes));
	bReturn &= Tweak((void*)qDisableRemnantMenuHook, pXorAlBytes, sizeof(pXorAlBytes));

	bReturn &= Tweak((void*)qEnableDebugFeature1, pMovAl01Bytes, sizeof(pMovAl01Bytes));
	bReturn &= Tweak((void*)qEnableDebugFeature2, pMovAl01Bytes, sizeof(pMovAl01Bytes));
	bReturn &= Tweak((void*)qEnableDebugFeature3, pMovAl01Bytes, sizeof(pMovAl01Bytes));

	bReturn &= Tweak((void*)qDisableDebugFeature1, pRetBytes, sizeof(pRetBytes));

	uint8_t pFreezeCamBytes[] = { 0xC6, 0x05, 0xEF, 0xED, 0x53, 0x03, 0x02, 0x8B, 0x83, 0xE0, 0x00, 0x00, 0x00, 0xFF, 0xC8, 0x83, 0xF8, 0x01, 0x0F, 0x87, 0x2C, 0x02, 0x00, 0x00, 0xC6, 0x05, 0xD7, 0xED, 0x53, 0x03, 0x01 };
	bReturn &= Tweak((void*)qEnableFreezeCam, pFreezeCamBytes, sizeof(pFreezeCamBytes));
	uint8_t pPanCamBytes[] = { 0xE8, 0x99, 0x06, 0xDB, 0x01 };
	bReturn &= Tweak((void*)qEnablePanCam, pPanCamBytes, sizeof(pPanCamBytes));

	return true;
};

bool CHooks::GetHookSites() {

	CSignature* Signature = GetSignature();

	if (!qMenuDrawHook) {
		qMenuDrawHook = Signature->GetSignature(&sMenuDrawHook);
		if (!qMenuDrawHook)
			return false;
		else
			qMenuDrawHook += 40;
	};
	if (!qMissingParamHook1) {
		qMissingParamHook1 = Signature->GetSignature(&sDisableMissingParam1);
		if (!qMissingParamHook1)
			return false;
		else
			qMissingParamHook1 += 9;
	};
	if (!qMissingParamHook2) {
		qMissingParamHook2 = Signature->GetSignature(&sDisableMissingParam2);
		if (!qMissingParamHook2)
			return false;
	};
	if (!qDisableRemnantMenuHook) {
		qDisableRemnantMenuHook = Signature->GetSignature(&sDisableRemnantMenu);
		if (!qDisableRemnantMenuHook)
			return false;
		else
			qDisableRemnantMenuHook += 49;
	};
	if (!qDisableDebugFeature1) {
		qDisableDebugFeature1 = Signature->GetSignature(&sSigDisableFeature1);
		if (!qDisableDebugFeature1)
			return false;
	};


	if (!qEnableFreezeCam) {
		qEnableFreezeCam = Signature->GetSignature(&sSigEnableFreezeCam);
		if (!qEnableFreezeCam)
			return false;
		else
			qEnablePanCam = qEnableFreezeCam - 83;
	};
	uint64_t qEnableDebugFeature = Signature->GetSignature(&sSigEnable3Areas);
	qEnableDebugFeature1 = qEnableDebugFeature + 8;
	qEnableDebugFeature2 = qEnableDebugFeature + 24;
	qEnableDebugFeature3 = qEnableDebugFeature + 40;


	return true;
};

bool CHooks::Hook(void* pHookSite, void* pDetour, DWORD64* pRetn, int iLen) {
	if (pRetn) *pRetn = (DWORD64)pHookSite + iLen;
	if (MH_CreateHook(pHookSite, pDetour, 0) != MH_OK) return false;
	if (MH_EnableHook(pHookSite)) return false;
	return true;
};

bool CHooks::Tweak(void* pMem, void* pNewBytes, int iLen) {
	DWORD dOldProtect;
	if (!VirtualProtect(pMem, iLen, PAGE_READWRITE, &dOldProtect)) return false;
	memcpy(pMem, pNewBytes, iLen);
	return VirtualProtect(pMem, iLen, dOldProtect, &dOldProtect);
};

bool CHooks::Hookless(void** pHookMem, void* pDetour, void** pOld) {
	DWORD dOldProtect;
	if (!VirtualProtect(pHookMem, 8, PAGE_READWRITE, &dOldProtect)) return false;
	*pOld = *pHookMem;
	*pHookMem = pDetour;
	return VirtualProtect(pHookMem, 8, dOldProtect, &dOldProtect);
};