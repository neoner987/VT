global _start
_start:
	mov rax, 0
	push rax
	push QWORD [rsp + 0]
	pop rax
	test rax, rax
	jz Label0
	mov rax, 4
	push rax
	push QWORD [rsp + 0]
	mov rax, 60
	pop rdi
	syscall
	add rsp, 8
	Label0:
	push QWORD [rsp + 0]
	mov rax, 60
	pop rdi
	syscall
	mov rax, 60
	mov rdi, 0
	syscall
