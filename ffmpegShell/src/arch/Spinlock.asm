
.code

; Note to myself : On x64 Windows,  rbx, rbp, rdi, rsi, r12-r15 are non volatile registers. 

AcquireSpinLock PROC

IFNDEF ARCH_64
	lea ecx, dword ptr[esp+8]
	xor ebx, ebx
	inc ebx
tryAcquire:
	xor eax, eax
	lock cmpxchg dword ptr[ecx], bl
	pause
	jnz tryAcquire
ELSE ;X64 BEGIN
	mov qword ptr[rsp+8], rcx
	xor r8,r8
	inc r8
tryAcquire:
	xor rax, rax
	lock cmpxchg qword ptr[rcx], r8b
	pause
	jnz tryAcquire
ENDIF

acquired:
	ret

AcquireSpinLock ENDP

ReleaseSpinLock PROC
IFNDEF ARCH_64
	btr dword ptr[esp+8], 0
ELSE
	mov qword ptr[rsp+8], rcx
	btr qword ptr[rcx], 0
ENDIF
	ret
ReleaseSpinLock ENDP

TestSpinLock PROC
IFNDEF ARCH_64
	xor eax, eax
	mov ebx, dword ptr[esp+8]
	test ebx, ebx
	jz notLocked
	inc eax
ELSE
	mov qword ptr[rsp+8], rcx
	xor rax,rax
	mov r8, qword ptr [rcx]
	test r8, r8
	jz notLocked
	inc rax
ENDIF
notLocked:
	pause
	ret
TestSpinLock ENDP

SpinWait PROC
IFNDEF ARCH_64
	mov ecx, dword ptr [esp+8]
ELSE
	mov qword ptr[rsp+8], rcx
ENDIF

spin:
	pause
	loop spin
	ret
SpinWait ENDP


END

