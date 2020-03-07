#pragma once
#include <Windows.h>
#include <Xinput.h> //Input From Xbox Controllers

//Defined Controller Buttons
#define GamePad_Left_Joystick_Click 22550
#define GamePad_Right_Joystick_Click 22551
#define GamePad_DPAD_UP 22544
#define GamePad_DPAD_DOWN 22545
#define GamePad_DPAD_LEFT 22546
#define GamePad_DPAD_RIGHT 22547
#define GamePad_Button_A 22528

//Structure For "XInputGetKeyStroke"
//_XINPUT_KEYSTROKE gamePadKeyData;

//Function To Check Controller State
bool checkControllerState();