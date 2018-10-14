.text

.globl hook_internal_trampoline
.type hook_internal_trampoline, @function
hook_internal_trampoline:
	mov  (%rsp), %rax
	mov  %rax, -16(%rsp)
	mov  8(%rsp), %rax
	mov  %rax, -8(%rsp)
	lea  trampoline(%rip), %rax
	mov  %rax, (%rsp)
	sub  $16, %rsp
	mov  hook_resolver@GOTPCREL(%rip), %rax
	mov  (%rax), %rax
	jmp  *%rax

.type trampoline, @function
trampoline:
	pop  %rdi
	push %rax
	mov  hook_installed@GOTPCREL(%rip), %rax
	mov  (%rax), %rax
	call *%rax
	pop  %rax
	ret
