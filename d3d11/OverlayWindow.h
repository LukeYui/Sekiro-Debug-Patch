#pragma once

#include "d3d11.h"
#include <string>
#include "XinHelper.h"

/******************************\

[TO USE THIS OVERLAY]
1. Change "procName" to overlay process name
2. Create a thread to "CreateOverlay"
3. Add a callback to OverlayCode in your main function (must run frequently to work properly)

/******************************/

//Global Vars
extern bool dllActive;
extern HWND overHwnd;
extern HDC windowDC;
extern bool overlayState;
extern bool updateOverlay;
extern LPCSTR procName;

//Functions To Create A Window
DWORD APIENTRY CreateOverlay(HINSTANCE hInstance);

ATOM MyRegisterClass(HINSTANCE hInstance);

HWND InitInstance(HINSTANCE, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Function To Modify Overlay
void CALLBACK OverlayCode();	//Main Overlay Code (Check Hotkeys & Modify Display)

void DrawString(int x, int y, int textW, int textH, HDC HDC_Desktop, COLORREF color, const char* text);		//Function To Draw Text

void OverlayWindowPos(HWND overlayWindow);		//Function To Update Position Of Overlay

void ClearOverlay(HWND overlayWindow);		//Function To Clear Overlay

void OverlayInput(DWORD input);		//Function To Handle Input From Hotkeys

void DrawMenu(char(*menuOptions)[32], LPCSTR menuName, int numOfOptions, int x, int y);		//Function To Draw A Menu Which Can Receive Input