section .text
global main
B1:
	mov dword [rbp + -4], 8
	jmp B2
B2:
	mov rax, 5
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov rax, 5
	mov rcx, rax
	call nothing
	mov rsp, rbp
	pop rbp
	pop r9
	pop r8
	pop rdx
	pop rcx
	mov rbx, rax
	pop rax
	sub rax, rbx
k:
	push rbp
	mov rbp, rsp
	sub rsp, 15
	mov dword [rbp + 16], ecx
	mov dword [rbp + 24], edx
	mov dword [rbp + 32], r8d
	mov dword [rbp + 40], r9d
	mov qword [rbp + -8], 25
	mov dword [rbp + -12], 19
	mov word [rbp + -14], 67
	mov byte [rbp + -15], 34
	add rsp, 15
	pop rbp
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 12
	mov dword [rbp + -4], 5
	mov qword [rbp + -12], 3
	add qword [rbp + -12], 4
	sub qword [rbp + -12], 6
	push rcx
	push rdx
	push r8
	push r9
	push rbp
	mov rbp, rsp
	sub rsp, 32
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push rbp
	mov rbp, rsp
	sub rsp, 32
	push rax
	pop rax
	mov rax, qword [rbp + -12]
	mov rcx, rax
	call nothing
	mov rsp, rbp
	pop rbp
	pop r9
	pop r8
	pop rdx
	pop rcx
	mov rbx, rax
	pop rax
	mov rax, rbx
	mov rcx, rax
	call nothing
	mov rsp, rbp
	pop rbp
	pop r9
	pop r8
	pop rdx
	pop rcx
	mov qword [rbp + -12], rax
	push rcx
	push rdx
	push r8
	push r9
	push rbp
	mov rbp, rsp
	sub rsp, 32
	push rax
	pop rax
	mov eax, dword [rbp + -4]
	mov rcx, rax
	call nothing
	mov rsp, rbp
	pop rbp
	pop r9
	pop r8
	pop rdx
	pop rcx
	add qword [rbp + -12], rax
	push rax
	pop rax
	mov eax, dword [rbp + -4]
	cmp rax, 0
	jne B1
	add rsp, 12
	pop rbp
	ret
nothing:
	push rbp
	mov rbp, rsp
	mov dword [rbp + 16], ecx
	push rax
	pop rax
	mov eax, dword [rbp + 16]
	push eax
	pop eax
	add eax, dword [rbp + 16]
	pop rbp
	ret