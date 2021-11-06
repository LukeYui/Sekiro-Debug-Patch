#include "../SekiroDebugMenu.h"

uint64_t CSignature::GetSignature(SSignature* pSig) {

	uint64_t qAddressEnd = qEndAddress - pSig->iLen;
	uint32_t iMaxLen = 0;

	uint8_t* pAddressIndex = reinterpret_cast<uint8_t*>(qStartAddress);
	uint8_t* pAddressIndexEnd = reinterpret_cast<uint8_t*>(qAddressEnd);
	while (pAddressIndex < pAddressIndexEnd) {
		uint32_t iSize = 0;

		for (INT i = 0; i < pSig->iLen; i++) {
			if (!((pAddressIndex[i] == (uint8_t)pSig->pSig[i]) || (pSig->pMask[i] == '?'))) break;
			iSize++;
		};

		if (iSize > iMaxLen) iMaxLen = iSize;
		if (iSize == pSig->iLen) return (uint64_t)pAddressIndex;

		pAddressIndex++;
	};

	return 0;
};
