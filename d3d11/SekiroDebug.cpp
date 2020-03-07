#include "SekiroDebug.h"

DWORD64 bSekiroDebugMenuPrint = 0;
DWORD64 bSekiroDebugGUIPrint = 0;

extern SSekiroDebug* SekiroDebugStruct;

extern SDrawStruct DrawStruct[300];
extern HDC wdc;

BYTE pNopBytes[5] = { 0x90, 0x90, 0x90, 0x90, 0x90, };
BYTE pRetBytes[1] = { 0xC3 };
BYTE pXorRaxBytes[5] = { 0x48, 0x31, 0xC0, 0x90, 0x90 };
BYTE pSetAlBytes[5] = { 0xB0, 0x01, 0x90, 0x90, 0x90 };
DWORD dHMAVCheck1 = 0x00CE9840;
DWORD dHMAVCheck2 = 0x036973AA;

BYTE pFreeCamBytes1[5] = { 0xE8, 0xBD, 0xB0, 0xD0, 01 };
BYTE pFreeCamBytes2[35] = { 0x44, 0x88, 0xA6, 0x98, 0x00, 0x00, 0x00, 0x8B, 0x83, 0xE0, 0x00, 0x00, 0x00, 0xFF, 0xC8, 0x83, 0xF8, 0x01, 0x0F, 0x87, 0x35, 0x01, 0x00, 0x00, 0x44, 0x88, 0xBE, 0x98, 0x00, 0x00, 0x00 };

VOID CSekiroDebug::Start() {
	Run();
	return;
};

VOID CSekiroDebug::Run() {

	Hook(HookSite_Menu, &bSekiroDebugMenuPrint, (DWORD64)&tSekiroDebugMenuPrint, 7);

	TweakMem(0x1424FCE20, 1, pRetBytes); //DebugMenu Font
	TweakMem(0x141B119A0, 1, pRetBytes); //DebugMenu Font 
	TweakMem(0x140595F30, 1, pRetBytes); //DebugMenu Font 
	TweakMem(0x1424E4370, 1, pRetBytes); //DebugMenu Font 
	TweakMem(0x1424E9340, 1, pRetBytes); //[[[143B858C0]+1D28]+70]+80 == NULL (CameraParam crash)

	TweakMem(0x14096B650, 2, pSetAlBytes); //GAME > INS
	TweakMem(0x14096B640, 2, pSetAlBytes); //GAME > Event

	while (SekiroDebugStruct->dIsActive) {
		SetUnhandledExceptionFilter(UHFilter);
		DrawStrings(0);
		ClearStrings();
		Sleep(5);
	};

	return;
};

BOOL CSekiroDebug::Hook(DWORD64 qAddress, DWORD64* pReturn, DWORD64 qDetour, DWORD dByteGap) {

	*pReturn = (qAddress + dByteGap);
	if (MH_CreateHook((LPVOID)qAddress, (LPVOID)qDetour, 0) != MH_OK) return false;
	if (MH_EnableHook((LPVOID)qAddress) != MH_OK) return false;

	return true;
};

BOOL CSekiroDebug::Unhook(DWORD64 qAddress) {
	if (MH_DisableHook((LPVOID)qAddress)) return false;
	return true;
};

BOOL CSekiroDebug::TweakMem(DWORD64 qAddress, DWORD dSize, VOID* pBytes) {

	DWORD dOldProtect = 0;

	if (!VirtualProtect((LPVOID)qAddress, dSize, PAGE_EXECUTE_READWRITE, &dOldProtect)) return false;
	memcpy((VOID*)qAddress, pBytes, dSize);
	VirtualProtect((LPVOID)qAddress, dSize, dOldProtect, &dOldProtect);

	return true;
};

VOID fSekiroDebugMenuPrint(SDebugPrint* D) {

	FLOAT fFontSize = 20.00f;

	SDebugPrint S = SDebugPrint();

	if (!D) return;
	if (!D->fX || !D->fY || !D->wcText[0]) return;

	memcpy(S.wcText, D->wcText, 150);

	for (int i = 0; i < MaxPrint; i++) {
		if (!DrawStruct[i].dIsActive) {
			DrawStruct[i].dIsActive = 1;
			DrawStruct[i].fFontSize = fFontSize;
			DrawStruct[i].sDebugPrint.fX = D->fX;
			DrawStruct[i].sDebugPrint.fY = D->fY;
			memcpy(DrawStruct[i].sDebugPrint.wcText, D->wcText, 250);
			break;
		};
	};

	return;
};

VOID fSekiroDebugGUIPrint(FLOAT fX, FLOAT fY, wchar_t* pwcText) {

	SDebugPrint D = SDebugPrint();

	if (!fX || !fY || !pwcText) return;

	D.fX = fX;
	D.fY = fY;
	
	memcpy(D.wcText, pwcText, 150);
	//Doesn't work for some reason.
	SekiroDebugStruct->pFontWrapper->DrawString(pContext, D.wcText, 20.00, D.fX, D.fY, C_WHITE, FW1_RESTORESTATE);

	return;
};

LONG WINAPI CSekiroDebug::UHFilter(struct _EXCEPTION_POINTERS* apExceptionInfo) {

	DWORD ExceptionCode = apExceptionInfo->ExceptionRecord->ExceptionCode;
	DWORD ExceptionFlags = apExceptionInfo->ExceptionRecord->ExceptionFlags;
	DWORD64 FaultOffset = (DWORD64)apExceptionInfo->ExceptionRecord->ExceptionAddress;
	DWORD NumberParameters = apExceptionInfo->ExceptionRecord->NumberParameters;
	FILE* fp;
	LPCWSTR ExceptionString = L"INVALID";

	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);

	switch (ExceptionCode) {
	case(EXCEPTION_ACCESS_VIOLATION): ExceptionString = L"ACCESS VIOLATION"; break;
	case(EXCEPTION_ARRAY_BOUNDS_EXCEEDED): ExceptionString = L"OUT OF BOUNDS"; break;
	case(EXCEPTION_BREAKPOINT): ExceptionString = L"BREAKPOINT REACHED"; break;
	case(EXCEPTION_DATATYPE_MISALIGNMENT): ExceptionString = L"MISALIGNED DATA"; break;
	case(EXCEPTION_FLT_DENORMAL_OPERAND): ExceptionString = L"FLOAT TOO SMALL"; break;
	case(EXCEPTION_FLT_DIVIDE_BY_ZERO): ExceptionString = L"FLOAT DIV 0"; break;
	case(EXCEPTION_FLT_INEXACT_RESULT): ExceptionString = L"FLOAT RESULT ERROR"; break;
	case(EXCEPTION_FLT_INVALID_OPERATION): ExceptionString = L"FLOAT ERROR"; break;
	case(EXCEPTION_FLT_OVERFLOW): ExceptionString = L"FLOAT OVERFLOW"; break;
	case(EXCEPTION_FLT_STACK_CHECK): ExceptionString = L"FLOAT DERIVED STACK ERROR"; break;
	case(EXCEPTION_FLT_UNDERFLOW): ExceptionString = L"FLOAT UNDERFLOW"; break;
	case(EXCEPTION_ILLEGAL_INSTRUCTION): ExceptionString = L"ILLEGAL INSTRUCTION"; break;
	case(EXCEPTION_IN_PAGE_ERROR): ExceptionString = L"PAGE ERROR"; break;
	case(EXCEPTION_INT_DIVIDE_BY_ZERO): ExceptionString = L"INT DIV 0"; break;
	case(EXCEPTION_INT_OVERFLOW): ExceptionString = L"INTEGER OVERFLOW"; break;
	case(EXCEPTION_INVALID_DISPOSITION): ExceptionString = L"INVALID DISPOSITION"; break;
	case(EXCEPTION_NONCONTINUABLE_EXCEPTION): ExceptionString = L"BYPASSED UNHANDLED EXCEPTION"; break;
	case(EXCEPTION_PRIV_INSTRUCTION): ExceptionString = L"PRIVATE INSTRUCTION"; break;
	case(EXCEPTION_SINGLE_STEP): ExceptionString = L"SINGLE STEP OPERATION COMPLETED"; break;
	case(EXCEPTION_STACK_OVERFLOW): ExceptionString = L"STACK OVERFLOW"; break;
	};

	wprintf_s(L"----- [YuiSpy Activated] -----\n\n");
	wprintf_s(L"Exception Code: 0x%X (%s)\n", ExceptionCode, ExceptionString);
	wprintf_s(L"Exception Flags: %X\n", ExceptionFlags);
	wprintf_s(L"Fault Offset: 0x%llX\n", FaultOffset);
	wprintf_s(L"Number Parameters: %X\n", NumberParameters);

	if (ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
		DWORD64 InaccessbileMem = (DWORD64)apExceptionInfo->ExceptionRecord->ExceptionInformation[1];
		if (apExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0) {
			wprintf_s(L"The instruction at 0x%llX referenced memory at 0x%llX. The memory could not be read.\n", FaultOffset, InaccessbileMem);
		}
		else {
			wprintf_s(L"The instruction at 0x%llX referenced memory at 0x%llX. The memory could not be written.\n", FaultOffset, InaccessbileMem);
		};
	};

	wprintf_s(L"Generating register dump:\n");
	wprintf_s(L"RAX: 0x%llX\n", apExceptionInfo->ContextRecord->Rax);
	wprintf_s(L"RBX: 0x%llX\n", apExceptionInfo->ContextRecord->Rbx);
	wprintf_s(L"RCX: 0x%llX\n", apExceptionInfo->ContextRecord->Rcx);
	wprintf_s(L"RDX: 0x%llX\n", apExceptionInfo->ContextRecord->Rdx);
	wprintf_s(L"RDI: 0x%llX\n", apExceptionInfo->ContextRecord->Rdi);
	wprintf_s(L"RSI: 0x%llX\n", apExceptionInfo->ContextRecord->Rsi);
	wprintf_s(L"R8:  0x%llX\n", apExceptionInfo->ContextRecord->R8);
	wprintf_s(L"R9:  0x%llX\n", apExceptionInfo->ContextRecord->R9);
	wprintf_s(L"R10: 0x%llX\n", apExceptionInfo->ContextRecord->R10);
	wprintf_s(L"R11: 0x%llX\n", apExceptionInfo->ContextRecord->R11);
	wprintf_s(L"R12: 0x%llX\n", apExceptionInfo->ContextRecord->R12);
	wprintf_s(L"R13: 0x%llX\n", apExceptionInfo->ContextRecord->R13);
	wprintf_s(L"R14: 0x%llX\n", apExceptionInfo->ContextRecord->R14);
	wprintf_s(L"R15: 0x%llX\n", apExceptionInfo->ContextRecord->R15);

	wprintf_s(L"RBP: 0x%llX\n", apExceptionInfo->ContextRecord->Rbp);
	wprintf_s(L"RSP: 0x%llX\n\n", apExceptionInfo->ContextRecord->Rsp);

	wprintf_s(L"------ [YuiSpy Complete] -----");

	MessageBoxA(NULL, "The application has crashed.", "YuiSpy", MB_ICONERROR);

	FreeConsole();

	exit(1);

};

BOOL CSekiroDebug::CheckPackedBytes(DWORD64 qAddress, WORD wBytes) {
	if (*(WORD*)qAddress == wBytes) return true;
	return false;
};

VOID CSekiroDebug::DrawStrings(IFW1FontWrapper* pFontWrapper) {

	for (int i = 0; i < MaxPrint; i++) {
		if (DrawStruct[i].dIsActive) {
			TextOutW(wdc, (DWORD)DrawStruct[i].sDebugPrint.fX, (DWORD)DrawStruct[i].sDebugPrint.fY, DrawStruct[i].sDebugPrint.wcText, (int)wcslen(DrawStruct[i].sDebugPrint.wcText));
		};
	};

	return;
};

VOID CSekiroDebug::ClearStrings() {

	for (int i = 0; i < MaxPrint; i++) {
		DrawStruct[i].dIsActive = 0;
	};

	return;
};