#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "..\..\ImGui/imgui.h"
#include "..\..\ImGui\backends\imgui_impl_win32.h"
#include "..\..\ImGui\backends\imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")

typedef HRESULT gfD3D11PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

struct SDebugPrintStruct {
	bool bIsActive = 0;
	float fX;
	float fY;
	wchar_t wText[1200];
};

class CGraphics {
public:
	bool HookD3D11Present();
	static HRESULT D3D11PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	virtual void DrawMenu();
	__declspec(noinline) SDebugPrintStruct* GetDebugPrintArrayStart();
	__declspec(noinline) int GetDebugPrintArraySize();
	__declspec(noinline) void EnterCS();
	__declspec(noinline) void LeaveCS();

private:
	bool bIsInitialised = false;
	gfD3D11PresentHook* fD3D11PresentHook = nullptr;
	float fWindowX = 0.00f;
	float fWindowY = 0.00f;
	CRITICAL_SECTION sDrawCS;

	//ImGui...
	ImGuiWindowFlags sWindowFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImFont* pDefaultFont = nullptr;
	ImFont* pMenuFont = nullptr;

	//Print
	SDebugPrintStruct sDebugMenuStruct[200];
};