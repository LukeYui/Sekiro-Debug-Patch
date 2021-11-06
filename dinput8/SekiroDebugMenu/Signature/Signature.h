#pragma once
#include <Windows.h>
#include <stdint.h>

struct SSignature {
	char* pSig;
	char* pMask;
	int iLen;
};

class CSignature {
public:
	__declspec(noinline) uint64_t GetSignature(SSignature* pSig);

	CSignature(const char* pModuleName) {
		hHandle = GetModuleHandleA(pModuleName);
		if (hHandle) {
			MEMORY_BASIC_INFORMATION sInfo;
			VirtualQuery(hHandle, &sInfo, sizeof(sInfo));
			IMAGE_DOS_HEADER* hDos = (IMAGE_DOS_HEADER*)hHandle;
			IMAGE_NT_HEADERS* hPe = (IMAGE_NT_HEADERS*)((ULONG64)sInfo.AllocationBase + (ULONG64)hDos->e_lfanew);
			qStartAddress = (uint64_t)hHandle;
			qEndAddress = qStartAddress + (uint64_t)hPe->OptionalHeader.SizeOfImage;
			CloseHandle(hHandle);
		};
	};

private:
	HANDLE hHandle = 0;
	uint64_t qStartAddress = 0;
	uint64_t qEndAddress = 0;
};
