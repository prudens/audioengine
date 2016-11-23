















































































































































































































































































































































































































































































.text
	.balign 4

.globl INT123_getcpuflags

INT123_getcpuflags:
	pushl %ebp
	movl %esp,%ebp
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %esi

	movl 8(%ebp), %esi

	movl $0x80000000,%eax

	pushfl
	pushfl
	popl %eax
	movl %eax,%ebx

	xorl $0x00200000,%eax
	pushl %eax
	popfl

	pushfl
	popl %eax
	popfl
	cmpl %ebx,%eax
	je .Lnocpuid


	movl $0x0, 12(%esi) 
	movl $0x0, 16(%esi) 

	movl $0x80000000, %eax
	cpuid

	cmpl $0x80000001, %eax
	jb .Lnoextended 

	movl $0x80000001,%eax
	cpuid
	movl %edx,12(%esi)
.Lnoextended:

	movl $0x00000001,%eax
	cpuid
	movl %eax, (%esi)
	movl %ecx, 4(%esi)
	movl %edx, 8(%esi)

	test	$0x04000000, %ecx
	jz		.Lend
	test	$0x08000000, %ecx
	jz		.Lend
	xor		%ecx, %ecx
	.byte	0x0f, 0x01, 0xd0 
	movl	%eax, 16(%esi)
	movl	(%esi), %eax
	jmp .Lend
	.balign 4
.Lnocpuid:

	movl $0, %eax
	movl $0, (%esi)
	movl $0, 4(%esi)
	movl $0, 8(%esi)
	movl $0, 12(%esi)
	movl $0, 16(%esi)
	.balign 4
.Lend:

	popl %esi
	popl %ebx
	popl %ecx
	popl %edx
	movl %ebp,%esp
	popl %ebp
	ret


