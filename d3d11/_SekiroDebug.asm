EXTERN bSekiroDebugMenuPrint:QWORD
EXTERN fSekiroDebugMenuPrint:PROC

EXTERN bSekiroDebugGUIPrint:QWORD
EXTERN fSekiroDebugGUIPrint:PROC

.data

.code

tSekiroDebugMenuPrint PROC

	mov edx,[rbx]
	mov rcx,rsi
	mov [rax],edx
	sub rsp,088h
	call _tSekiroDebugMenuPrint
	add rsp,088h

	Return:
	jmp [bSekiroDebugMenuPrint]

tSekiroDebugMenuPrint ENDP

_tSekiroDebugMenuPrint PROC


	mov [rsp+08h],rax
	mov [rsp+010h],rbx
	mov [rsp+018h],rcx
	mov [rsp+020h],rdx
	mov [rsp+028h],rbp
	push r8
	push r9
	push rdi
	push rsi
	sub rsp,060h
	mov rbp,rsp
	and rsp,-010h
	movaps [rsp+010h],xmm0
	movaps [rsp+020h],xmm1
	movaps [rsp+030h],xmm2
	mov rcx,rax
	call fSekiroDebugMenuPrint
	
	Return:
	movaps xmm0,[rsp+010h]
	movaps xmm1,[rsp+020h]
	movaps xmm2,[rsp+030h]
	mov rsp,rbp
	mov rax,[rsp+088h]
	mov rbx,[rsp+090h]
	mov rcx,[rsp+098h]
	mov rdx,[rsp+0A0h]
	mov rbp,[rsp+0A8h]
	add rsp,060h
	pop rsi
	pop rdi
	pop r9
	pop r8
	ret

_tSekiroDebugMenuPrint ENDP

tSekiroDebugGUIPrint PROC

	sub rsp,088h
	call _tSekiroDebugGUIPrint
	add rsp,088h

	Return:
	movss xmm0,DWORD PTR [rax+08h]
	jmp [bSekiroDebugGUIPrint]

tSekiroDebugGUIPrint ENDP

_tSekiroDebugGUIPrint PROC

	mov [rsp+08h],rax
	mov [rsp+010h],rbx
	mov [rsp+018h],rcx
	mov [rsp+020h],rdx
	mov [rsp+028h],rbp
	push r8
	push r9
	push rdi
	push rsi
	sub rsp,060h
	mov rbp,rsp
	and rsp,-010h
	movaps [rsp+010h],xmm0
	movaps [rsp+020h],xmm1
	movaps [rsp+030h],xmm2
	mov r8,rbx
	movss xmm0,DWORD PTR [rax]
	movss xmm1,DWORD PTR [rax+04h]
	call fSekiroDebugGUIPrint
	
	Return:
	movaps xmm0,[rsp+010h]
	movaps xmm1,[rsp+020h]
	movaps xmm2,[rsp+030h]
	mov rsp,rbp
	mov rax,[rsp+088h]
	mov rbx,[rsp+090h]
	mov rcx,[rsp+098h]
	mov rdx,[rsp+0A0h]
	mov rbp,[rsp+0A8h]
	add rsp,060h
	pop rsi
	pop rdi
	pop r9
	pop r8
	ret

_tSekiroDebugGUIPrint ENDP

END