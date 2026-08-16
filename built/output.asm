global _start
_start:
	mov rax, 5
	push rax
	mov rax, 20
	push rax
	mov rax, 7
	push rax
	push QWORD [rsp + 0]

	push QWORD [rsp + 16]

	pop rax
	pop rbx
	sub rax, rbx
	push rax
	push QWORD [rsp + 24]

	pop rax
	pop rbx
	add rax, rbx
	push rax
	push QWORD [rsp + 0]

	mov rax, 60
	pop rdi
	syscall
	mov rax, 60
	mov rdi, 0
	syscall
