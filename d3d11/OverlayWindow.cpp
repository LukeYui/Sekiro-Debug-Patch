#include "OverlayWindow.h"
#include "SekiroDebug.h"

WCHAR szTitle[100];
WCHAR szWindowClass[100];

//Global Var Redefinitions
HWND overHwnd;
HDC windowDC;
bool dllActive = TRUE;
bool updateOverlay = TRUE;
LPCSTR procName = "Sekiro";

//Global Vars For Drawing
bool overlayState = true; //Displaying Overlay

//Structure For "XInputGetKeyStroke"
_XINPUT_KEYSTROKE gamePadKeyData;

//Defines For Used Gamepad Conditions
#define GP_A_Down 22528 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_B_Down 22529 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_X_DOWN 22530 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Y_DOWN 22531 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Select_Down 22549 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Dpad_Up_Down 22544 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Dpad_Down_Down 22545 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Dpad_Left_Down 22546 + XINPUT_KEYSTROKE_KEYDOWN
#define GP_Dpad_Right_Down 22547 + XINPUT_KEYSTROKE_KEYDOWN

#define GP_A_Repeat 22528 + XINPUT_KEYSTROKE_REPEAT
#define GP_B_Repeat 22529 + XINPUT_KEYSTROKE_REPEAT
#define GP_X_Repeat 22530 + XINPUT_KEYSTROKE_REPEAT
#define GP_Y_Repeat 22531 + XINPUT_KEYSTROKE_REPEAT
#define GP_Select_Repeat 22549 + XINPUT_KEYSTROKE_REPEAT
#define GP_Dpad_Up_Repeat 22544 + XINPUT_KEYSTROKE_REPEAT
#define GP_Dpad_Down_Repeat 22545 + XINPUT_KEYSTROKE_REPEAT
#define GP_Dpad_Left_Repeat 22546 + XINPUT_KEYSTROKE_REPEAT
#define GP_Dpad_Right_Repeat 22547 + XINPUT_KEYSTROKE_REPEAT

#define RB 22532 + XINPUT_KEYSTROKE_KEYDOWN
#define RB_R 22532 + 5
#define LS 22563 + XINPUT_KEYSTROKE_KEYDOWN
#define LS_R 22563 + XINPUT_KEYSTROKE_REPEAT

/******************************\

[ALL VARS BELOW ARE FOR OPTIONAL MENU]

/******************************/

char scriptMenu[3][32] = {
	"Menu Option 1",
	"Menu Option 2",
	"Menu Option 3",
};

bool scriptStates[3] = {false, false, false};

//Menu Control Vars
bool displayMenu = false;
int menuNr = 0;
int menuIndex = 0;
int curMenuMax = 0;

DWORD APIENTRY CreateOverlay(HINSTANCE hInstance)
{
	HFONT hFont = 0;

	//Set Strings
	wcscpy_s(szTitle, L"External Overlay");
	wcscpy_s(szWindowClass, L"OVERLAYCLASS");

	//Iniitiate Window Class
	MyRegisterClass(hInstance);

	//Create Window
	overHwnd = InitInstance(hInstance, 5);

	// Perform application initialization:
	if (!overHwnd)
		return 0x2;

	//Get Drawing Context Of Window
	windowDC = GetDC(overHwnd);

	//Edit Text
	SetBkColor(windowDC, RGB(0, 0, 0));
	SetBkMode(windowDC, TRANSPARENT);
	SetTextColor(windowDC, RGB(255, 255, 255));

	hFont = CreateFontW(30, 14, 0, 0, FW_DONTCARE,
		0, 0, 0, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, VARIABLE_PITCH,
		TEXT(L"Courier New")/*"SYSTEM_FIXED_FONT"*/);

	SelectObject(windowDC, hFont);

	while (dllActive)
	{
		//Get Messages To Keep Window Active
		MSG msg;

		// Main message loop:
		if (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Sleep(10);
	}

	return TRUE;
}

void CALLBACK OverlayCode(int iIsAutoRefresh)
{
	//Overlay Position Update
	OverlayWindowPos(overHwnd);

	if (iIsAutoRefresh) {
		ClearOverlay(overHwnd);
		return;
	};

	if (checkControllerState())
	{
		XInputGetKeystroke(0, 0, &gamePadKeyData);

		WORD result = gamePadKeyData.VirtualKey + gamePadKeyData.Flags;

		switch (result)
		{
		case GP_A_Down:
		case GP_B_Down:
		case GP_X_DOWN:
		case GP_Y_DOWN:
		case GP_X_Repeat:
		case GP_Y_Repeat:
		case GP_Select_Down:
		case GP_Select_Repeat:
		case GP_Dpad_Up_Down:
		case GP_Dpad_Down_Down:
		case GP_Dpad_Left_Down:
		case GP_Dpad_Right_Down:
		case GP_Dpad_Down_Repeat:
		case GP_Dpad_Right_Repeat:
			ClearOverlay(overHwnd);
			break;
		default:
			break;
		}

	}
	else
	{
		Sleep(100);
		ClearOverlay(overHwnd);
	}
}

void OverlayWindowPos(HWND overlayWindow)
{
	//Get Process
	HWND procToOverlay = FindWindowA(NULL, procName);

	if (!procToOverlay)
		return;

	RECT windowLocation;
	
	GetWindowRect(procToOverlay, &windowLocation);

	int width = windowLocation.right - windowLocation.left;
	int height = windowLocation.bottom - windowLocation.top;

	if (GetForegroundWindow() == procToOverlay) {
		ShowWindow(overlayWindow, SW_SHOW);
		SetWindowPos(overlayWindow, HWND_TOPMOST, windowLocation.left, windowLocation.top, width, height, 0x200);
	}
	else ShowWindow(overlayWindow, SW_HIDE);

	return;
}

void ClearOverlay(HWND overlayWindow)
{
	RECT windowLocation;

	GetWindowRect(overlayWindow, &windowLocation);

	//Clear Text
	InvalidateRect(overlayWindow, &windowLocation, TRUE);

	//Brief Sleep To Prevent Interference W/ Next Draw
	Sleep(1);

	return;
}

void DrawString(int x, int y, int textW, int textH, HDC HDC_Desktop, COLORREF color, const char* text)
{
	HFONT Font = CreateFontA(textH, textW, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, NULL);

	SetTextAlign(HDC_Desktop, TA_LEFT); //TA_CENTER | TA_NOUPDATECP);

	SetBkColor(HDC_Desktop, RGB(0, 0, 0));
	SetBkMode(HDC_Desktop, TRANSPARENT);

	SetTextColor(HDC_Desktop, color);

	SelectObject(HDC_Desktop, Font);

	TextOutA(HDC_Desktop, x, y, text, strlen(text));

	DeleteObject(Font);
}

void OverlayInput(DWORD input)
{
	//Check Input
	switch (input)
	{
	//Overlay Inputs
	case 0:				//Basic Draw Update
		break;
	case 1:				//Toggle Overlay State
		overlayState = !overlayState;
		break;
	case 2:
		displayMenu = !displayMenu;
		break;
	//Menu Inputs
	case 0xF1:
		if(menuIndex != curMenuMax)
			menuIndex++;
		break;
	case 0xF2:
		if(menuIndex != 0)
			menuIndex--;
		break;
	default:
		char drawStr[40] = "Error! The Input(";
		strcat_s(drawStr, 40 , std::to_string(input).c_str());
		strcat_s(drawStr, 40 , ") Has No Input Case!");

		DrawString(200, 100, 15, 20, windowDC, RGB(255, 0, 0), (LPCSTR)drawStr);
		break;
	}

	//Get Overlay To Update
	updateOverlay = TRUE;
	
	return;
}

void DrawMenu(char(*menuOptions)[32], LPCSTR menuName, int numOfOptions, int x, int y)
{
	//Draw Menu Name
	//char * tempStr = new char[33];
	//strcpy_s(tempStr, 33, menuName);
	//strcat_s(tempStr, 33, ":");
	//DrawString(x, y, 10, 15, windowDC, RGB(0, 255, 255), menuName);

	//delete[] tempStr;

	//Display Menu
	for (int i = 0; i < numOfOptions; i++)
	{
		y += 20;

		COLORREF drawColor;

		if (i == menuIndex)
			drawColor = RGB(0, 255, 0);
		else
			drawColor = RGB(255, 0, 0);

		DrawString(x, y, 10, 15, windowDC, drawColor, menuOptions[i]);

		//Draw Enabled State
		if (scriptStates[i])
		{
			drawColor = RGB(0, 255, 0);
			DrawString((x + 150), y, 10, 15, windowDC, drawColor, "[ENABLED]");
		}
		else
		{
			drawColor = RGB(255, 0, 0);
			DrawString((x + 150), y, 10, 15, windowDC, drawColor, "[DISABLED]");
		}
	}

	//Save Current Menu Max
	curMenuMax = (numOfOptions - 1);

	return;
}

/******************************\

[WINDOW CREATION FUNCTIONS BELOW]

/******************************/

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTGUIPROJECT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)2; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = L"App Menu"; //MAKEINTRESOURCEW(IDC_TESTGUIPROJECT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	//Create Window & Save
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	//Make Window Transparent
	SetWindowLongW(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	//Check Windwo State & Update
	if (!hWnd)
		return NULL;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//Test Draw On Opening
	//HDC hdcTest = GetDC(hWnd);
	//COLORREF myColor = 0x00FF0000;

	//DrawString(15, 15, 20, 20, hdcTest, myColor, "[Overlay Initial Text Draw]");

	return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}