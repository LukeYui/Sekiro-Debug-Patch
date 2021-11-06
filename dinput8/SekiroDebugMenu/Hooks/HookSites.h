#pragma once
#include <Windows.h>

//struct SSignature;

SSignature sActivateDebugMenu = {
	"\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x32\xC0\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xB8",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	(int)strlen(sActivateDebugMenu.pMask),
};

SSignature sMenuDrawHook = {
	"\x38\x4C\x8B\xC0\xF3\x0F\x11\x44\x24\x2C",
	"xxxxxxxxxx",
	(int)strlen(sMenuDrawHook.pMask),
};

SSignature sDisableMissingParam1 = {
	"\x02\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x8B\xC4\x55\x57",
	"xxxxxxxxxxxxxx",
	(int)strlen(sDisableMissingParam1.pMask),
};

SSignature sDisableMissingParam2 = {
	"\x40\x55\x56\x57\x41\x56\x41\x57\x48\x8D\x6C\x24\xC9\x48\x81\xEC\xE0\x00\x00\x00\x48\xC7\x44",
	"xxxxxxxxxxxxxxxxxxxxxxx",
	(int)strlen(sDisableMissingParam2.pMask),
};

SSignature sDisableRemnantMenu = {
	"\xC7\x44\x24\x38\x29\x00\x00\x00\x48\x89\x5C\x24\x40\x48\x8D\x4C\x24\x30\x88\x88\x88\x88\x88\xB9",
	"xxxxxxxxxxxxxxxxxx?????x",
	(int)strlen(sDisableRemnantMenu.pMask),
};

SSignature sSigEnable3Areas = {
	"\x48\x83\xC4\x70\x5B\xC3\xCC\xCC\x32\xC0\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x32",
	"xxxxxxxxxxxxxxxxxxxxxxxxx",
	(int)strlen(sSigEnable3Areas.pMask),
};

//Disable..

SSignature sSigDisableFeature1 = {
	"\x48\x89\x5C\x24\x20\x55\x48\x83\xEC\x20\x48\x83\xB9",
	"xxxxxxxxxxxxx",
	(int)strlen(sSigDisableFeature1.pMask),
};

//Camera
SSignature sSigEnableFreezeCam = {
	"\x8B\x83\xE0\x00\x00\x00\xFF\xC8\x83",
	"xxxxxxxxx",
	(int)strlen(sSigEnableFreezeCam.pMask),
};