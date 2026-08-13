global _start
_start:
	mov rax, 69
	push rax
	mov rax, 67
	push rax
	push QWORD [rsp + 0]

	mov rax, 60
	pop rdi
	syscall
	mov rax, 60
	mov rdi, 0
	syscall
