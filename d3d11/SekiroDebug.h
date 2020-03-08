#pragma once
#include "d3d11.h"
#include "OverlayWindow.h"

#define HookSite_Menu 0x1424F9D6D

#define MaxPrint 299

struct SDebugPrint;

class CSekiroDebug {
public:
	virtual VOID Start();
	static LONG WINAPI UHFilter(struct _EXCEPTION_POINTERS* apExceptionInfo);
	virtual VOID Run();
	virtual BOOL Hook(DWORD64 qAddress, DWORD64* pReturn, DWORD64 qDetour, DWORD dByteGap);
	virtual BOOL Unhook(DWORD64 qAddress);
	virtual BOOL TweakMem(DWORD64 qAddress, DWORD dSize, VOID* pBytes);
	virtual BOOL ApplyPatches();
	virtual BOOL CheckPackedBytes(DWORD64 qAddress, WORD wBytes);
	virtual VOID DrawStrings(IFW1FontWrapper* pFontWrapper);
	virtual VOID ClearStrings();
	virtual VOID UpdateOverlayWindow(int iIsAutoUpdate);
	virtual VOID MinimiseShelvesOptions(FLOAT* pFontSize, DWORD dY);
	virtual VOID GetUserPreferences();
};

struct SSekiroDebug {
	DWORD dIsActive = 1;
	IFW1FontWrapper* pFontWrapper;
};

struct SDebugPrint {
	FLOAT fX;
	FLOAT fY;
	BYTE pUnk[0x10];
	wchar_t wcText[300];
};

struct SDrawStruct {
	DWORD dIsActive;
	FLOAT fFontSize;
	SDebugPrint sDebugPrint;
};

extern "C" VOID tSekiroDebugMenuPrint();
extern "C" DWORD64 bSekiroDebugMenuPrint;
extern "C" VOID fSekiroDebugMenuPrint(SDebugPrint* D);

extern "C" VOID tSekiroDebugGUIPrint();
extern "C" DWORD64 bSekiroDebugGUIPrint;
extern "C" VOID fSekiroDebugGUIPrint(FLOAT fX, FLOAT fY, wchar_t* pwcText);