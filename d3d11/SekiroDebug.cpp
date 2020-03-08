#include "SekiroDebug.h"

DWORD64 bSekiroDebugMenuPrint = 0;
DWORD64 bSekiroDebugGUIPrint = 0;

extern CSekiroDebug* SekiroDebug;
extern SSekiroDebug* SekiroDebugStruct;

extern SDrawStruct DrawStruct[300];
extern HDC wdc;

BYTE pNopBytes[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
BYTE pRetBytes[1] = { 0xC3 };
BYTE pXorRaxBytes[5] = { 0x48, 0x31, 0xC0, 0x90, 0x90 };
BYTE pSetAlBytes[5] = { 0xB0, 0x01, 0x90, 0x90, 0x90 };
BYTE pSetR8lBytes[5] = { 0x41, 0xB0, 0x01 };
DWORD dHMAVCheck1 = 0x00CE9840;
DWORD dHMAVCheck2 = 0x036973AA;

BYTE pFreeCamBytes1[5] = { 0xE8, 0xBD, 0xB0, 0xD0, 01 };
BYTE pFreeCamBytes2[35] = { 0x44, 0x88, 0xA6, 0x98, 0x00, 0x00, 0x00, 0x8B, 0x83, 0xE0, 0x00, 0x00, 0x00, 0xFF, 0xC8, 0x83, 0xF8, 0x01, 0x0F, 0x87, 0x35, 0x01, 0x00, 0x00, 0x44, 0x88, 0xBE, 0x98, 0x00, 0x00, 0x00 };

char* pGameUserPrefStrings[80];
BYTE pGameUserPref[80];
DWORD64 pGameUserPrefHooks[80];

VOID CSekiroDebug::Start() {
	Run();
	return;
};

VOID CSekiroDebug::Run() {

	Hook(HookSite_Menu, &bSekiroDebugMenuPrint, (DWORD64)&tSekiroDebugMenuPrint, 7);

	if (!ApplyPatches()) {
		TextOutW(windowDC, 80, 80, L"Initialisation Failed", 22);
		SekiroDebugStruct->dIsActive = 0;
		return;
	};

	while (SekiroDebugStruct->dIsActive) {
		SetUnhandledExceptionFilter(UHFilter);
		DrawStrings(0);
		UpdateOverlayWindow(false);
		ClearStrings();
		Sleep(20);
	};

	return;
};

BOOL CSekiroDebug::ApplyPatches() {
	return 
		TweakMem(0x1424FCE20, 1, pRetBytes) && //DebugMenu Font
		TweakMem(0x141B119A0, 1, pRetBytes) && //DebugMenu Font 
		TweakMem(0x140595F30, 1, pRetBytes) && //DebugMenu Font 
		TweakMem(0x1424E4370, 1, pRetBytes) && //DebugMenu Font 
		TweakMem(0x1424E9340, 1, pRetBytes) && //[[[143B858C0]+1D28]+70]+80 == NULL (CameraParam crash)
		TweakMem(0x14096B650, 2, pSetAlBytes) && //GAME > INS
		TweakMem(0x14096B640, 2, pSetAlBytes); //GAME > Event
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
	if (D->fX < 1.00f || D->fY < 1.00f || !D->wcText[0]) return;

	if (SekiroDebug) SekiroDebug->MinimiseShelvesOptions(&fFontSize, (DWORD)D->fY);

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
			//TextOutW(wdc, (DWORD)DrawStruct[i].sDebugPrint.fX, (DWORD)DrawStruct[i].sDebugPrint.fY, DrawStruct[i].sDebugPrint.wcText, (int)wcslen(DrawStruct[i].sDebugPrint.wcText));
			TextOutW(windowDC, (DWORD)DrawStruct[i].sDebugPrint.fX, (DWORD)DrawStruct[i].sDebugPrint.fY, DrawStruct[i].sDebugPrint.wcText, (int)wcslen(DrawStruct[i].sDebugPrint.wcText));
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


VOID CSekiroDebug::UpdateOverlayWindow(int iIsAutoUpdate) {
	OverlayCode(iIsAutoUpdate);
	return;
};

VOID CSekiroDebug::MinimiseShelvesOptions(FLOAT* pFontSize, DWORD dY) {

	//I've had to manually do this bit to look slick

	switch (dY) {
	case(80): {
		*pFontSize = 18.00f;
		return;
	};
	case(67): {
		*pFontSize = 16.00f;
		return;
	};
	case(56): {
		*pFontSize = 14.00f;
		return;
	};
	case(48): {
		*pFontSize = 12.00f;
		return;
	};
	case(42): {
		*pFontSize = 10.00f;
		return;
	};
	case(36): {
		*pFontSize = 8.00f;
		return;
	};
	case(32): {
		*pFontSize = 7.00f;
		return;
	};
	default: return;
	};

	return;
};

VOID CSekiroDebug::GetUserPreferences() {

	INIReader SReader("SekiroDebug.ini");

	if (SReader.ParseError()) return;

	for (int i = 0; i < 68; i++) {
		pGameUserPref[i] = SReader.GetBoolean("GAME", (char*)&pGameUserPrefStrings[i], false);
	};



};

extern char* pGameUserPrefStrings[80] = {
	"Game.DisableNowLoading",
	"Game.EnterButtonIsCircle",
	"Game.Network.AcceptForceJoin",
	"Game.SaveData.BindSaveDataEnable",
	"GameData.AllNoArrowConsume",
	"GameData.AllNoAttack",
	"GameData.AllNoDamage",
	"GameData.AllNoDead",
	"GameData.AllNoGoodsConsume",
	"GameData.AllNoHit",
	"GameData.AllNoMagicQtyConsume",
	"GameData.AllNoMove",
	"GameData.AllNoMpConsume",
	"GameData.AllNoResourceItemConsume",
	"GameData.AllNoStaminaConsume",
	"GameData.DisableFallDamage",
	"GameData.EnableCreateCharaBack",
	"GameData.EnableDeleteCharaBack",
	"GameData.EnablePlayerReload",
	"GameData.DbgDrawEnemyHpGauge",
	"GameData.DbgDrawEnemySpGauge",
	"GameData.UseBattleFindPathWhenCautionState",
	"GameData.UseBattleFindPathWhenFindState",
	"GameData.PlayerExterminate",
	"GameData.PlayerExterminateStamina",
	"GameData.PlayerNoDead",
	"GameData.PlayerNoGoodsConsume",
	"GameData.PlayerNoResouceItemConsume",
	"GameData.PlayerNoRevivalConsume",
	"GameData.TaeDebugDisableDeadFlag",
	"GameData.TaeDebugDisableRagdoll",
	"GameData.TaeDebugEnableAnimePlaySpeed",
	"GameData.TaeDebugEnableBehavior",
	"GameData.TaeDebugEnableMovement",
	"GameData.TaeDebugEnableTestParam",
	"Game.Debug.LimitedLoadFileVersion",
	"Game.Debug.GetOverrideUnlockedType_forDLC2",
	"Game.Debug.GetOverrideUnlockedType_forDLC1",
	"Game.Sfx.Debug.DefaultInfoLvl",
	"Game.Camera.Obstacle.EnableFade",
	"Game.IsBackgroundFrameSkip",
	"GameRule.IsDbgMaxBaseEquipWeight",
	"GameRule.IsDbgNoDurabilityLossWeaponProtector",
	"Game.DbgSwimModeChangeCtrlEnable",
	"Game.IsDebugCameraStopModeOnly",
	"Game.Debug.IsDisableCultExecuteCheck",
	"Game.IsDisablePlayAnimMapEnter",
	"Game.IsE3Mode",
	"Game.Event.EnableDebugEventBreak",
	"Game.Debug.IsEnableDefaultBonfireMenu",
	"Game.Debug.IsEnableFrameSkipOnDisactiveWindow",
	"Game.Player.WireAction.EnableIngameCreateEdge",
	"Game.Debug.EnableRegulationFile",
	"Game.IsGamescom",
	"Game.IsLimitedResourceItem",
	"GameRule.IsNoArtsPointConsume",
	"Game.EnablePerfMan",
	"Game.IsProtoAutoEquipMode",
	"Game.IsProtoPresenMode",
	"Game.IsTGS",
	"Game.Event.TrueDead",
	"Game.IsUnifiedControllerForPS4",
	"Game.IsUseChrFollowCam",
	"Game.Debug.IsUseEbl",
	"Game.Debug.IsUseEblAlias",
	"Game.IsUseSPChrExFollowCam",
	"Game.IsVFRMode",

};
extern BYTE pGameUserPref[80] = { 0 };
extern DWORD64 pGameUserPrefHooks[80] = {
	0x140961573, // [bool] - Game.DisableNowLoading
	0x140961A3E, // [bool] - Game.EnterButtonIsCircle
	0x140961B4B, // [bool] - Game.Network.AcceptForceJoin
	0x140961DF3, // [bool] - Game.SaveData.BindSaveDataEnable
	0x14096308E, // [bool] - GameData.AllNoArrowConsume
	0x14096319E, // [bool] - GameData.AllNoAttack
	0x1409632AE, // [bool] - GameData.AllNoDamage
	0x1409633BE, // [bool] - GameData.AllNoDead
	0x1409634CE, // [bool] - GameData.AllNoGoodsConsume
	0x1409635DE, // [bool] - GameData.AllNoHit
	0x1409636EE, // [bool] - GameData.AllNoMagicQtyConsume
	0x1409637FE, // [bool] - GameData.AllNoMove
	0x14096390E, // [bool] - GameData.AllNoMpConsume
	0x140963A1E, // [bool] - GameData.AllNoResourceItemConsume
	0x140963B2E, // [bool] - GameData.AllNoStaminaConsume
	0x140963C3E, // [bool] - GameData.DisableFallDamage
	0x140963D4E, // [bool] - GameData.EnableCreateCharaBack
	0x140963E5E, // [bool] - GameData.EnableDeleteCharaBack
	0x140963F6E, // [bool] - GameData.EnablePlayerReload
	0x1409640B3, // [bool] - GameData.DbgDrawEnemyHpGauge
	0x140964203, // [bool] - GameData.DbgDrawEnemySpGauge
	0x140964353, // [bool] - GameData.UseBattleFindPathWhenCautionState (Already True)
	0x1409644A3, // [bool] - GameData.UseBattleFindPathWhenFindState (Already True)
	0x1409645BE, // [bool] - GameData.PlayerExterminate
	0x1409646CE, // [bool] - GameData.PlayerExterminateStamina
	0x1409647DE, // [bool] - GameData.PlayerNoDead
	0x1409648EE, // [bool] - GameData.PlayerNoGoodsConsume
	0x1409649FE, // [bool] - GameData.PlayerNoResouceItemConsume
	0x140964B0E, // [bool] - GameData.PlayerNoRevivalConsume
	0x140964C1E, // [bool] - GameData.TaeDebugDisableDeadFlag
	0x140964D2E, // [bool] - GameData.TaeDebugDisableRagdoll
	0x140964E3E, // [bool] - GameData.TaeDebugEnableAnimePlaySpeed
	0x140964F4E, // [bool] - GameData.TaeDebugEnableBehavior
	0x14096505E, // [bool] - GameData.TaeDebugEnableMovement
	0x14096516E, // [bool] - GameData.TaeDebugEnableTestParam
	0x140965C03, // [bool] - Game.Debug.LimitedLoadFileVersion
	0x140968689, // [bool] - Game.Debug.GetOverrideUnlockedType_forDLC2
	0x1409687A1, // [bool] - Game.Debug.GetOverrideUnlockedType_forDLC1
	0x140968DF3, // [bool] - Game.Sfx.Debug.DefaultInfoLvl
	0x140736432, // [bool] - Game.Camera.Obstacle.EnableFade (Already True)
	0x14096A3D3, // [bool] - Game.IsBackgroundFrameSkip
	0x14096A8DB, // [bool] - GameRule.IsDbgMaxBaseEquipWeight
	0x14096AA2E, // [bool] - GameRule.IsDbgNoDurabilityLossWeaponProtector
	0x14096AB93, // [bool] - Game.DbgSwimModeChangeCtrlEnable
	0x14096ACE3, // [bool] - Game.IsDebugCameraStopModeOnly
	0x14096AF6B, // [bool] - Game.Debug.IsDisableCultExecuteCheck
	0x14096B0B3, // [bool] - Game.IsDisablePlayAnimMapEnter
	0x14096B213, // [bool] - Game.IsE3Mode
	0x14096B5D3, // [bool] - Game.Event.EnableDebugEventBreak
	0x14096B743, // [bool] - Game.Debug.IsEnableDefaultBonfireMenu
	0x14096B9E3, // [bool] - Game.Debug.IsEnableFrameSkipOnDisactiveWindow
	0x14096BAFB, // [bool] - Game.Player.WireAction.EnableIngameCreateEdge
	0x14096BC4B, // [bool] - Game.Debug.EnableRegulationFile
	0x14096BF41, // [bool] - Game.IsGamescom
	0x14096C0A3, // [bool] - Game.IsLimitedResourceItem
	0x14096C1FE, // [bool] - GameRule.IsNoArtsPointConsume
	0x14096C30E, // [bool] - Game.EnablePerfMan
	0x14096C453, // [bool] - Game.IsProtoAutoEquipMode
	0x14096C5A3, // [bool] - Game.IsProtoPresenMode
	0x14096CAE3, // [bool] - Game.IsTGS
	0x14096CC33, // [bool] - Game.Event.TrueDead
	0x14096CD83, // [bool] - Game.IsUnifiedControllerForPS4
	0x14096D023, // [bool] - Game.IsUseChrFollowCam
	0x14096D2C3, // [bool] - Game.Debug.IsUseEbl
	0x14096D46E, // [bool] - Game.Debug.IsUseEblAlias
	0x14096D883, // [bool] - Game.IsUseSPChrExFollowCam
	0x14096D99E, // [bool] - Game.IsVFRMode
};