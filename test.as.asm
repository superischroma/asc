section .text
global main
k:
	push rbp
	mov rbp, rsp
	sub rsp, 15
	mov dword [rbp + 16], ecx
	mov dword [rbp + 24], edx
	mov dword [rbp + 32], r8d
	mov dword [rbp + 40], r9d
	mov qword [rbp - 8], 25
	mov dword [rbp - 12], 19
	mov word [rbp - 14], 67
	mov byte [rbp - 15], 34
	add rsp, 15
	pop rbp
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 12
	mov dword [rbp - 4], 5
	mov qword [rbp - 12], 3
	add qword [rbp - 12], 4
	sub qword [rbp - 12], 6
	mov eax, 5
	sub eax, 5
	add rsp, 12
	pop rbp
	ret