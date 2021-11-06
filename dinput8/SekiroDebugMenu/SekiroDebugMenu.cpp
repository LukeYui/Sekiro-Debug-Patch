#include "SekiroDebugMenu.h"

CSekiroDebugMenu* SekiroDebugMenu = nullptr;
CHooks* Hooks = nullptr;
CSignature* Signature = nullptr;
CGraphics* Graphics = nullptr;

bool Initialise() {

	SekiroDebugMenu = new CSekiroDebugMenu();
	Hooks = SekiroDebugMenu->Hooks;
	Signature = SekiroDebugMenu->Signature;
	Graphics = SekiroDebugMenu->Graphics;

	if (!Hooks->Patch())
		return false;

	if (!Graphics->HookD3D11Present())
		return false;

	return true;
};

CHooks* GetHooks() {
	if (!Hooks) {

	};
	return Hooks;
};
CSignature* GetSignature() {
	if (!Signature) {

	};
	return Signature;
};
CGraphics* GetGraphics() {
	if (!Graphics) {

	};
	return Graphics;
};
void InitDebug() {
#ifdef DEBUG
	FILE* fp;
	AllocConsole();
	SetConsoleTitleA("Sekiro - Debug Console");
	freopen_s(&fp, "CONOUT$", "w", stdout);
	DebugPrint(L"Starting...");
#endif
	return;
};

void DebugPrint(wchar_t* pwcMsg, ...) {
#ifdef DEBUG
	va_list vl;
	va_start(vl, pwcMsg);
	vwprintf_s(pwcMsg, vl);
	va_end(vl);
	wprintf_s(L"\n");
#endif
	return;
};

void Panic(const wchar_t* pcError, const wchar_t* pcMsg, int iError) {

	wchar_t pBuffer[256];

	DebugPrint(L"[Engine] Panic");
	DebugPrint(pBuffer);

	if (*(int*)0 == iError) {
		exit(0);
	};
	return;
};