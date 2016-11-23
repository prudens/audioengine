






























































































































































































































































































































































































































































































.data

.section .rodata

	.balign 8
.LC0:
	.long 0x0,0x40dfffc0
	.balign 8
.LC1:
	.long 0x0,0xc0e00000
	.balign 8
.text

.globl INT123_synth_1to1_i586_asm
INT123_synth_1to1_i586_asm:
	subl $12,%esp
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx

	movl 32(%esp),%eax 
	movl 40(%esp),%esi 
	movl 48(%esp),%edi 
	movl (%edi),%ebp   
	xorl %edi,%edi
	cmpl %edi,36(%esp)
	jne .L48           
	decl %ebp          
	andl $15,%ebp      
	movl 48(%esp),	%edi 
	movl %ebp,(%edi)   
	xorl %edi,%edi     
	movl 44(%esp),%ecx 
	jmp .L49
.L48: 
	addl $2,%esi
	movl 44(%esp),%ecx 
	addl $2176,%ecx
.L49:
	testl $1,%ebp
	je .L50
	movl %ecx,%ebx
	movl %ebp,16(%esp)
	pushl %eax
	movl 20(%esp),%edx
	leal (%ebx,%edx,4),%eax
	pushl %eax
	movl 24(%esp),%eax
	incl %eax
	andl $15,%eax
	leal 1088(,%eax,4),%eax
	addl %ebx,%eax
	jmp .L74
.L50:
	leal 1088(%ecx),%ebx
	leal 1(%ebp),%edx
	movl %edx,16(%esp)
	pushl %eax
	leal 1092(%ecx,%ebp,4),%eax
	pushl %eax
	leal (%ecx,%ebp,4),%eax
.L74:
	pushl %eax
	call INT123_dct64_i386
	addl $12,%esp

	movl 16(%esp),%edx
	leal 0(,%edx,4),%edx
	movl 52(%esp),%eax 
	addl $64,%eax
	movl %eax,%ecx
	subl %edx,%ecx
	movl $16,%ebp
.L55:
	flds (%ecx)
	fmuls (%ebx)
	flds 4(%ecx)
	fmuls 4(%ebx)
	fxch %st(1)
	flds 8(%ecx)
	fmuls 8(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 12(%ecx)
	fmuls 12(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 16(%ecx)
	fmuls 16(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 20(%ecx)
	fmuls 20(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 24(%ecx)
	fmuls 24(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 28(%ecx)
	fmuls 28(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 32(%ecx)
	fmuls 32(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 36(%ecx)
	fmuls 36(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 40(%ecx)
	fmuls 40(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 44(%ecx)
	fmuls 44(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 48(%ecx)
	fmuls 48(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 52(%ecx)
	fmuls 52(%ebx)
	fxch %st(2)         
	faddp %st,%st(1)
	flds 56(%ecx)
	fmuls 56(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds 60(%ecx)
	fmuls 60(%ebx)
	fxch %st(2)
	subl $4,%esp
	faddp %st,%st(1)
	fxch %st(1)
	fsubrp %st,%st(1)
	fistpl (%esp)
	popl %eax
	cmpl $32767,%eax
	jg 1f
	cmpl $-32768,%eax
	jl 2f
	movw %ax,(%esi)
	jmp 4f
1:	movw $32767,(%esi)
	jmp 3f
2:	movw $-32768,(%esi)
3:	incl %edi
4:
.L54:
	addl $64,%ebx
	subl $-128,%ecx
	addl $4,%esi
	decl %ebp
	jnz .L55
	flds (%ecx)
	fmuls (%ebx)
	flds 8(%ecx)
	fmuls 8(%ebx)
	flds 16(%ecx)
	fmuls 16(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 24(%ecx)
	fmuls 24(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 32(%ecx)
	fmuls 32(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 40(%ecx)
	fmuls 40(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 48(%ecx)
	fmuls 48(%ebx)
	fxch %st(2)
	faddp %st,%st(1)
	flds 56(%ecx)
	fmuls 56(%ebx)
	fxch %st(2)
	subl $4,%esp
	faddp %st,%st(1)
	fxch %st(1)
	faddp %st,%st(1)
	fistpl (%esp)
	popl %eax
	cmpl $32767,%eax
	jg 1f
	cmpl $-32768,%eax
	jl 2f
	movw %ax,(%esi)
	jmp 4f
1:	movw $32767,(%esi)
	jmp 3f
2:	movw $-32768,(%esi)
3:	incl %edi
4:
.L62:
	addl $-64,%ebx
	addl $4,%esi
	movl 16(%esp),%edx
	leal -128(%ecx,%edx,8),%ecx
	movl $15,%ebp
.L68:
	flds -4(%ecx)
	fchs
	fmuls (%ebx)
	flds -8(%ecx)
	fmuls 4(%ebx)
	fxch %st(1)
	flds -12(%ecx)
	fmuls 8(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -16(%ecx)
	fmuls 12(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -20(%ecx)
	fmuls 16(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -24(%ecx)
	fmuls 20(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -28(%ecx)
	fmuls 24(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -32(%ecx)
	fmuls 28(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -36(%ecx)
	fmuls 32(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -40(%ecx)
	fmuls 36(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -44(%ecx)
	fmuls 40(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -48(%ecx)
	fmuls 44(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -52(%ecx)
	fmuls 48(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -56(%ecx)
	fmuls 52(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds -60(%ecx)
	fmuls 56(%ebx)
	fxch %st(2)
	fsubrp %st,%st(1)
	flds (%ecx)
	fmuls 60(%ebx)
	fxch %st(2)
	subl $4,%esp
	fsubrp %st,%st(1)
	fxch %st(1)
	fsubrp %st,%st(1)
	fistpl (%esp)
	popl %eax
	cmpl $32767,%eax
	jg 1f
	cmpl $-32768,%eax
	jl 2f
	movw %ax,(%esi)
	jmp 4f
1:	movw $32767,(%esi)
	jmp 3f
2:	movw $-32768,(%esi)
3:	incl %edi
4:
.L67:
	addl $-64,%ebx
	addl $-128,%ecx
	addl $4,%esi
	decl %ebp
	jnz .L68
	movl %edi,%eax
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	addl $12,%esp
	ret


