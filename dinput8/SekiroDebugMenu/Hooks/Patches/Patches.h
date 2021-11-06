#pragma once
#include <Windows.h>

struct SMenuDrawLocation {
	float f1;
	float f2;
	float f3;
	float f4;
};

void DrawMenuHook(uint64_t qUnkClass, SMenuDrawLocation* pLocationData, wchar_t* pwString);