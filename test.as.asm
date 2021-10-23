section .text
global main
main:
	push dword 5
	push dword 0
	pop rax
	ret
nothing:
	push dword 5
	push dword 2
	pop rax
	ret