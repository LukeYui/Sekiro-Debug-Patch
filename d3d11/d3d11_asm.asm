
extern mProcs:QWORD

.data

	pSwapChain dq 0

.code

_D3D11On12CreateDevice PROC

	jmp mProcs[0*8]

_D3D11On12CreateDevice ENDP

_D3D11CoreRegisterLayers PROC

	jmp mProcs[1*8]

_D3D11CoreRegisterLayers ENDP

_D3D11CreateDevice PROC

	jmp mProcs[2*8]

_D3D11CreateDevice ENDP

_D3D11CreateDeviceAndSwapChain PROC

	jmp mProcs[3*8]

_D3D11CreateDeviceAndSwapChain ENDP

_EnableFeatureLevelUpgrade PROC

	jmp mProcs[4*8]

_EnableFeatureLevelUpgrade ENDP

_D3D11CoreCreateDevice PROC

	jmp mProcs[5*8]

_D3D11CoreCreateDevice ENDP

_D3D11CoreCreateLayeredDevice PROC

	jmp mProcs[6*8]

_D3D11CoreCreateLayeredDevice ENDP

_D3D11CoreGetLayeredDeviceSize PROC

	jmp mProcs[7*8]

_D3D11CoreGetLayeredDeviceSize ENDP

END