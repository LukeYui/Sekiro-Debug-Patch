#pragma once
#include "../MinHook/include/MinHook.h"

#include "Hooks/Hooks.h"
#include "Signature/Signature.h"
#include "Graphics/Graphics.h"

__declspec(noinline) bool Initialise();
__declspec(noinline) void InitDebug();
__declspec(noinline) void DebugPrint(wchar_t* pwcMsg, ...);
__declspec(noinline) void Panic(const wchar_t* pcError, const wchar_t* pcMsg, int iError);

CHooks* GetHooks();
CSignature* GetSignature();
CGraphics* GetGraphics();

class CSekiroDebugMenu {
public:
	CHooks* Hooks = new CHooks();
	CSignature* Signature = new CSignature("sekiro.exe");
	CGraphics* Graphics = new CGraphics();

};