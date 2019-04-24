.section .rodata

.globl p
p:
.quad rbp
.quad rbpe

.globl rbp
rbp:
	mov  (%rbp), %rbp
rbpe:

.globl x
x:
.quad rax
.quad rbx
.quad rcx
.quad rdx
.quad rdxe

.globl rax
rax:
	mov  (%rax), %rax

.globl rbx
rbx:
	mov  (%rbx), %rbx

.globl rcx
rcx:
	mov  (%rcx), %rcx

.globl rdx
rdx:
	mov  (%rdx), %rdx
rdxe:

.globl i
i:
.quad rdi
.quad rsi
.quad rsie

.globl rdi
rdi:
	mov  (%rdi), %rdi

.globl rsi
rsi:
	mov  (%rsi), %rsi
rsie:

.globl nul
nul:
.quad r8
.quad r9
.quad r9e

.globl r8
r8:
	mov  (%r8), %r8

.globl r9
r9:
	mov  (%r9), %r9
r9e:

.globl one
one:
.quad r10
.quad r11
.quad r12
.quad r13
.quad r14
.quad r15
.quad r15e

.globl r10
r10:
	mov  (%r10), %r10

.globl r11
r11:
	mov  (%r11), %r11

.globl r12
r12:
	mov  (%r12), %r12

.globl r13
r13:
	mov  (%r13), %r13

.globl r14
r14:
	mov  (%r14), %r14

.globl r15
r15:
	mov  (%r15), %r15
r15e:
