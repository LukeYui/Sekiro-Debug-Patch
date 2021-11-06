#include "../SekiroDebugMenu.h"
#include <string>
#include <locale>
#include <codecvt>

static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
ID3D11Texture2D* RenderTargetTexture;
ID3D11RenderTargetView* RenderTargetView = NULL;
DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pDeviceVTable = NULL;

HWND window = nullptr;
static WNDPROC OriginalWndProcHandler = nullptr;

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CGraphics::HookD3D11Present() {

	HMODULE hDXGI = 0;
	while (!hDXGI) {
		hDXGI = GetModuleHandleA("dxgi.dll");
		Sleep(4000);
	};

	IDXGISwapChain* pSwapChain;
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	window = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = window;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;
	IDXGISwapChain* d3dSwapChain = 0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		MessageBoxA(window, "Failed to create directX device and swapchain!", "Error", MB_ICONERROR);
		return false;
	}

	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];
	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	CGraphics* Graphics = GetGraphics();

	GetHooks()->Hookless((void**)&pSwapChainVtable[8], CGraphics::D3D11PresentHook, (void**)&Graphics->fD3D11PresentHook);

	return true;
};


HRESULT CGraphics::D3D11PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {

	CGraphics* Graphics = GetGraphics();

	if (!Graphics->bIsInitialised) {

		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}

		DXGI_SWAP_CHAIN_DESC sd;
		pSwapChain->GetDesc(&sd);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		window = sd.OutputWindow;

		OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(pDevice, pContext);
		ImGui::GetIO().ImeWindowHandle = window;

		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&RenderTargetTexture)))
		{
			pDevice->CreateRenderTargetView(RenderTargetTexture, NULL, &RenderTargetView);
			RenderTargetTexture->Release();
		}

		//CreateFont
		Graphics->pDefaultFont = io.Fonts->AddFontDefault();

		io.Fonts->GetGlyphRangesDefault();

		static const ImWchar RANGES[] = {
			0x0020, 0x007F, // Basic Latin
			0x00A0, 0x00FF, // Latin-1 Supplement
			0x2000, 0x206F, // General Punctuation
			0x2191, 0x2191, // General Punctuation
			0x226A, 0x226B,  // Much greater than/less than symbol
			0x25A0, 0x26C6, // Black box - White Diamond
			0x3000, 0x303F, // CJK Symbols and Punctuation
			0x3040, 0x309F, // Hiragana
			0x30A0, 0x30FF, // Katakana
			0x31F0, 0x31FF, // Katakana Phonetic Extensions
			0x4E00, 0x9FFF, // CJK Unified Ideographs
			0xFF00, 0xFFEF, // Halfwidth and Fullwidth Forms
			0,
		};

		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = 4.0f;
		Graphics->pMenuFont = io.Fonts->AddFontFromFileTTF("debugfont.ttf", 20.0f, &font_cfg, RANGES);

		RECT rect;
		if (GetWindowRect(window, &rect)) {
			Graphics->fWindowX = rect.right - rect.left;
			Graphics->fWindowY = rect.bottom - rect.top;
		};


		InitializeCriticalSection(&Graphics->sDrawCS);
		Graphics->bIsInitialised = true;
	};

	pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

	//imgui
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	Graphics->DrawMenu();

	//Wrap Up ImGui
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return Graphics->fD3D11PresentHook(pSwapChain, SyncInterval, Flags);
};

//Menu man 0x143D67368
void CGraphics::DrawMenu() {

	EnterCS();

	ImGui::SetNextWindowSize(ImVec2(fWindowX, fWindowY));
	ImGui::Begin("DebugMenu", false, sWindowFlags);

	int iArraySize = GetDebugPrintArraySize();
	SDebugPrintStruct* pStruct = GetDebugPrintArrayStart();

	for (int i = 0; i < iArraySize; i++, pStruct++) {
		if (!pStruct->bIsActive) continue;

		std::wstring wCvt(pStruct->wText);
		std::string str(wCvt.begin(), wCvt.end());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		std::string dest = convert.to_bytes(wCvt);

		ImGui::GetWindowDrawList()->AddText(pMenuFont, 20.0f, ImVec2(pStruct->fX, pStruct->fY), ImColor(255, 255, 255, 255), dest.c_str(), 0, 0.0f, 0);
		pStruct->bIsActive = false;
	};

	ImGui::End();

	LeaveCS();
	return;
};

SDebugPrintStruct* CGraphics::GetDebugPrintArrayStart() {
	return sDebugMenuStruct;
};

int CGraphics::GetDebugPrintArraySize() {
	return sizeof(sDebugMenuStruct) / sizeof(SDebugPrintStruct);
};

void CGraphics::EnterCS() {
	EnterCriticalSection(&sDrawCS);
	return;
};
void CGraphics::LeaveCS() {
	LeaveCriticalSection(&sDrawCS);
	return;
};

bool ShowMenu = true;
LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(window, &mPos);
	ImGui::GetIO().MousePos.x = (float)mPos.x;
	ImGui::GetIO().MousePos.y = (float)mPos.y;

	if (uMsg == WM_KEYUP)
	{
		if (wParam == VK_INSERT)
		{
			if (ShowMenu)
				io.MouseDrawCursor = true;
			else
				io.MouseDrawCursor = false;
		}

	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}
