.code32
.section .data

	format_string: .string "val = <%i>\n"

.section .text

.global A

A:
	subl $4, %esp
	mov $5, %ebx
	mov %ebx, (%esp)
	addl $4, %esp	# Fixing the stack
	ret

.global Program4

Program4:
	subl $13, %esp
	mov $1, %bl
	mov %bl, 12(%esp)
	mov $1, %ebx
	mov $2, %ecx
	addl %ebx, %ecx
	mov %ecx, %eax
	mov %eax, 4(%esp)
	mov $3, %ebx
	addl 4(%esp), %ebx
	mov %ebx, %eax
	mov %eax, (%esp)
	mov (%esp), %eax
	mov %eax, 8(%esp)
	push %eax
	lea format_string, %edi
	push %edi
	call printf
	pop %eax
	pop %eax
	call A
	addl $13, %esp	# Fixing the stack
	ret

.global main

main:
	call Program4
	ret

/* C-Mod compiler: <i686>:<pc-linux-gnu> */

