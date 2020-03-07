#pragma once
#include <vector>
#include <d3d11.h>
#include <D3D11Shader.h>
#include <D3Dcompiler.h>

#include "MinHook/include/MinHook.h"
#include "FW1FontWrapper/FW1FontWrapper.h"

#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

extern DWORD_PTR* pSwapChainVtable;
extern ID3D11Device* pDevice;
extern ID3D11DeviceContext* pContext;
extern IDXGISwapChain* pSwapChain;

#define C_WHITE 0xFFFFFFFF  // RGBA White
#define COLOUR 0xFFbfb300
#define C_SHADOW 0xFF101010 // Shadow to improve readability
