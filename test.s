	.file	"test.c"
	.intel_syntax noprefix
	.text
	.globl	k
	.def	k;	.scl	2;	.type	32;	.endef
	.seh_proc	k
k:
	push	rbp
	.seh_pushreg	rbp
	mov	rbp, rsp
	.seh_setframe	rbp, 0
	sub	rsp, 16
	.seh_stackalloc	16
	.seh_endprologue
	mov	DWORD PTR 16[rbp], ecx
	mov	DWORD PTR 24[rbp], edx
	mov	DWORD PTR 32[rbp], r8d
	mov	DWORD PTR 40[rbp], r9d
	mov	DWORD PTR -4[rbp], 19
	mov	WORD PTR -6[rbp], 67
	mov	BYTE PTR -7[rbp], 34
	movsx	edx, BYTE PTR -7[rbp]
	movsx	eax, WORD PTR -6[rbp]
	add	eax, edx
	add	rsp, 16
	pop	rbp
	ret
	.seh_endproc
	.def	__main;	.scl	2;	.type	32;	.endef
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	push	rbp
	.seh_pushreg	rbp
	mov	rbp, rsp
	.seh_setframe	rbp, 0
	sub	rsp, 64
	.seh_stackalloc	64
	.seh_endprologue
	call	__main
	mov	DWORD PTR -4[rbp], 5
	mov	QWORD PTR -16[rbp], 7
	mov	DWORD PTR 32[rsp], 1
	mov	r9d, 0
	mov	r8d, 0
	mov	edx, 0
	mov	ecx, 0
	call	k
	add	rsp, 64
	pop	rbp
	ret
	.seh_endproc
	.ident	"GCC: (GNU) 11.2.0"
