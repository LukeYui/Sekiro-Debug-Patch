#include <windows.h>
#include <stdio.h>
#include "SekiroDebug.h"
#include "OverlayWindow.h"
#include "d3d11.h"

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

D3D11PresentHook phookD3D11Present = NULL;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
IDXGISwapChain *pSwapChain;

DWORD_PTR* pSwapChainVtable = NULL;

IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

ID3D11Texture2D* RenderTargetTexture;
ID3D11RenderTargetView* RenderTargetView = NULL;

HINSTANCE mHinst = 0, mHinstDLL = 0;

extern "C" UINT_PTR mProcs[51] = {0};

BOOL firstTime = TRUE;
BOOL bIsLaunchHook = TRUE;

HDC wdc;
RECT R;

CSekiroDebug* SekiroDebug;
SSekiroDebug* SekiroDebugStruct;

SDrawStruct DrawStruct[300];

VOID InitSekiroDebug();

extern BYTE pSetAlBytes[5];

LPCSTR mImportNames[8] = { 
	"D3D11On12CreateDevice",
	"D3D11CoreRegisterLayers",
	"D3D11CreateDevice",
	"D3D11CreateDeviceAndSwapChain",
	"EnableFeatureLevelUpgrade",
	"D3D11CoreCreateDevice",
	"D3D11CoreCreateLayeredDevice",
	"D3D11CoreGetLayeredDeviceSize",
};

const int MultisampleCount = 1; // Set to 1 to disable multisampling

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	HWND hWindow = 0;
	int iStrLen = 0;

	if (firstTime)

	{
		//get device and context
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}


		//create font
		HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);

		FW1_FONTWRAPPERCREATEPARAMS createParams;
		ZeroMemory(&createParams, sizeof(createParams));

		createParams.GlyphSheetWidth = 512;
		createParams.GlyphSheetHeight = 512;
		createParams.MaxGlyphCountPerSheet = (2048 * 10);
		createParams.SheetMipLevels = 1;
		createParams.AnisotropicFiltering = FALSE;
		createParams.MaxGlyphWidth = 384;
		createParams.MaxGlyphHeight = 384;
		createParams.DisableGeometryShader = FALSE;
		createParams.VertexBufferSize = 0;
		createParams.DefaultFontParams.pszFontFamily = L"Meiryo";
		createParams.DefaultFontParams.FontWeight = DWRITE_FONT_WEIGHT_NORMAL;
		createParams.DefaultFontParams.FontStyle = DWRITE_FONT_STYLE_NORMAL;
		createParams.DefaultFontParams.FontStretch = DWRITE_FONT_STRETCH_NORMAL;
		createParams.DefaultFontParams.pszLocale = L"JP";

		hResult = pFW1Factory->CreateFontWrapper(pDevice, NULL, &createParams, &pFontWrapper);

		pFW1Factory->Release();

		// use the back buffer address to create the render target
		//if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&RenderTargetTexture))))
		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&RenderTargetTexture)))
		{
			pDevice->CreateRenderTargetView(RenderTargetTexture, NULL, &RenderTargetView);
			RenderTargetTexture->Release();
		}

		wdc = GetDC(FindWindowA(0, "Sekiro"));

		firstTime = false;
	}



	//call before you draw
	pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

	BOOL oDisplayTime = TRUE;

	if (!pFontWrapper) return phookD3D11Present(pSwapChain, SyncInterval, Flags);

	if (bIsLaunchHook) {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitSekiroDebug, 0, 0, 0);
		bIsLaunchHook = false;
		SekiroDebugStruct->pFontWrapper = pFontWrapper;
	};

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);

};

VOID InitSekiroDebug() {
	SekiroDebug->Start();
	return;
};

DWORD BeginMainHook() {

	WORD wExpectedBytes = 0;
	DWORD64 qEnableDebugHookAddress = 0;

	SekiroDebug = new CSekiroDebug();
	SekiroDebugStruct = new SSekiroDebug();

	//We need to hook this right here, really early on
	qEnableDebugHookAddress = 0x14113DDD0;
	wExpectedBytes = 0xC032;
	
	//Sekiro is encrypted, so we need to wait for it to unencrypt itself.
	while (!SekiroDebug->CheckPackedBytes(qEnableDebugHookAddress, wExpectedBytes)) {

	};

	//Once it has, we know the encryption is gone so we can run this
	SekiroDebug->TweakMem(0x14113DDD0, 2, pSetAlBytes);
	SekiroDebug->GetUserPreferences();

	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle("dxgi.dll");

		Sleep(4000);

	}
	
	while (!hDXGIDLL); {

		Sleep(100);
	
	};

	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

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
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = MultisampleCount;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	// LibOVR 0.4.3 requires that the width and height for the backbuffer is set even if
	// you use windowed mode, despite being optional according to the D3D11 documentation.
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
		MessageBox(hWnd, "Failed to create directX device and swapchain!", "Error", MB_ICONERROR);
		return NULL;
	}

	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hookD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK) { return 1; }

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	while (true) {
		SekiroDebug->UpdateOverlayWindow(true);
		Sleep(1000);
	}

	if (pDevice)
		pDevice->Release();
	if (pContext)
		pContext->Release();
	if (pSwapChain)
		pSwapChain->Release();

	return NULL;

};

DWORD WINAPI Begind3d11Hook(LPVOID lpParam) {


	char dllpath[MAX_PATH];

	GetSystemDirectory(dllpath, MAX_PATH);

	strcat_s(dllpath, "\\d3d11.dll");

	mHinstDLL = LoadLibrary(dllpath);

	if (!mHinstDLL) {

		MessageBoxA(NULL, "Failed to load original DLL", "Error", MB_ICONERROR);

		return FALSE;

	};

	for (int i = 0; i < 8; i++) {

		mProcs[i] = (UINT_PTR)GetProcAddress(mHinstDLL, mImportNames[i]);

		if (mProcs[i] == NULL) {
		
			MessageBoxA(NULL, mImportNames[i], "Error", MB_ICONERROR);
		
		};

	};

	BeginMainHook();

	return FALSE;

};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		DisableThreadLibraryCalls(hinstDLL);

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Begind3d11Hook, NULL, NULL, NULL);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CreateOverlay, NULL, NULL, NULL);

		break;

	case DLL_PROCESS_DETACH:

		FreeLibrary(mHinstDLL);

		break;
	}

	return TRUE;

}

extern "C" void _D3D11On12CreateDevice();
extern "C" void _D3D11CoreRegisterLayers();
extern "C" void _D3D11CreateDevice();
extern "C" void _D3D11CreateDeviceAndSwapChain();
extern "C" void _EnableFeatureLevelUpgrade();
extern "C" void _D3D11CoreCreateDevice();
extern "C" void _D3D11CoreCreateLayeredDevice();
extern "C" void _D3D11CoreGetLayeredDeviceSize();