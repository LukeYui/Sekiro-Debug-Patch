#pragma once
#include <Windows.h>
#include <stdint.h>

#include "Patches/Patches.h"

class CHooks {
public:
	__declspec(noinline) bool Patch();
	__declspec(noinline) bool GetHookSites();
	virtual bool Hook(void* pHookSite, void* pDetour, DWORD64* pRetn, int iLen);
	virtual bool Tweak(void* pMem, void* pNewBytes, int iLen);
	virtual bool Hookless(void** pHookMem, void* pDetour, void** pOld);

private:
	//Memory...
	bool bIsMHActive = MH_Initialize();
	uint64_t qActivateDebugMenu = 0;
	uint64_t qMenuDrawHook = 0;
	uint64_t qMissingParamHook1 = 0;
	uint64_t qMissingParamHook2 = 0;
	uint64_t qDisableRemnantMenuHook = 0;

	//Enable...
	uint64_t qEnableDebugFeature1 = 0;
	uint64_t qEnableDebugFeature2 = 0;
	uint64_t qEnableDebugFeature3 = 0;

	//Disable..
	uint64_t qDisableDebugFeature1 = 0; // GUI

	//Cam
	uint64_t qEnableFreezeCam = 0;
	uint64_t qEnablePanCam = 0;
};