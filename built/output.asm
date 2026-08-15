global _start
_start:
	mov rax, 4
	push rax
	mov rax, 5
	push rax
	mov rax, 10
	push rax
	push QWORD [rsp + 16]

	push QWORD [rsp + 16]

	push QWORD [rsp + 16]

	pop rax
	pop rbx
	imul rax, rbx
	push rax
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
