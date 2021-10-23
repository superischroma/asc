a:
	push	rbp
	mov	rbp, rsp
	mov	DWORD PTR 16[rbp], ecx
	mov	DWORD PTR 24[rbp], edx
	mov	DWORD PTR 32[rbp], r8d
	mov	DWORD PTR 40[rbp], r9d
	mov	eax, 10
	pop	rbp
	ret
main:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 112
	call	__main
	mov	DWORD PTR 96[rsp], 120
	mov	DWORD PTR 88[rsp], 110
	mov	DWORD PTR 80[rsp], 100
	mov	DWORD PTR 72[rsp], 90
	mov	DWORD PTR 64[rsp], 80
	mov	DWORD PTR 56[rsp], 70
	mov	DWORD PTR 48[rsp], 60
	mov	DWORD PTR 40[rsp], 50
	mov	DWORD PTR 32[rsp], 40
	mov	r9d, 30
	mov	r8d, 20
	mov	edx, 10
	mov	ecx, 5
	call	a
	mov	eax, 0
	add	rsp, 112
	pop	rbp
	ret
