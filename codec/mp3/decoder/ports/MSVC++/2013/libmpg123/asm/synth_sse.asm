
































































































































































































































































































































































































































































































	.data
	.balign 8
one_null:
	.long	-65536
	.long	-65536
	.balign 8
null_one:
	.long	65535
	.long	65535

	.text
	.balign 16
	
.globl synth_1to1_sse_asm
synth_1to1_sse_asm:
	pushl	%ebp

	movl	%esp, %ebp

	subl  $4,%esp 
	pushl	%edi
	pushl	%esi
	pushl	%ebx


	movl 12(%ebp),%ecx
	movl 16(%ebp),%edi
	movl $15,%ebx
	movl 24(%ebp),%edx
	leal (%edi,%ecx,2),%edi
	decl %ecx
	movl 20(%ebp),%esi
	movl (%edx),%eax
	jecxz 1f
	decl %eax
	andl %ebx,%eax
	leal 1088(%esi),%esi
	movl %eax,(%edx)
1:
	leal (%esi,%eax,2),%edx
	movl %eax,12(%esp)
	incl %eax
	andl %ebx,%eax
	leal 544(%esi,%eax,2),%ecx
	incl %ebx
	testl $1, %eax
	jnz 2f
	xchgl %edx,%ecx
	incl 12(%esp)
	leal 544(%esi),%esi
2:
	pushl 8(%ebp)
	pushl %edx
	pushl %ecx
	call INT123_dct64_sse
	addl $12, %esp
	leal 1(%ebx), %ecx
	subl 12(%esp),%ebx
	pushl %ecx
	
	movl 28(%ebp),%ecx
	leal (%ecx,%ebx,2), %edx
	movl (%esp),%ecx 
	shrl $1, %ecx
	.balign 16
3:
	movq  (%edx),%mm0
	movq  64(%edx),%mm4
	pmaddwd (%esi),%mm0
	pmaddwd 32(%esi),%mm4
	movq  8(%edx),%mm1
	movq  72(%edx),%mm5
	pmaddwd 8(%esi),%mm1
	pmaddwd 40(%esi),%mm5
	movq  16(%edx),%mm2
	movq  80(%edx),%mm6
	pmaddwd 16(%esi),%mm2
	pmaddwd 48(%esi),%mm6
	movq  24(%edx),%mm3
	movq  88(%edx),%mm7
	pmaddwd 24(%esi),%mm3
	pmaddwd 56(%esi),%mm7
	paddd %mm1,%mm0
	paddd %mm5,%mm4
	paddd %mm2,%mm0
	paddd %mm6,%mm4
	paddd %mm3,%mm0
	paddd %mm7,%mm4
	movq  %mm0,%mm1
	movq  %mm4,%mm5
	psrlq $32,%mm1
	psrlq $32,%mm5
	paddd %mm1,%mm0
	paddd %mm5,%mm4
	psrad $13,%mm0
	psrad $13,%mm4
	packssdw %mm0,%mm0
	packssdw %mm4,%mm4
	movq	(%edi), %mm1
	punpckldq %mm4, %mm0
	pand   one_null, %mm1
	pand   null_one, %mm0
	por    %mm0, %mm1
	movq   %mm1,(%edi)
	leal 64(%esi),%esi
	leal 128(%edx),%edx
	leal 8(%edi),%edi
	decl %ecx
	jnz  3b
	popl %ecx
	andl $1, %ecx
	jecxz 4f
	movq  (%edx),%mm0
	pmaddwd (%esi),%mm0
	movq  8(%edx),%mm1
	pmaddwd 8(%esi),%mm1
	movq  16(%edx),%mm2
	pmaddwd 16(%esi),%mm2
	movq  24(%edx),%mm3
	pmaddwd 24(%esi),%mm3
	paddd %mm1,%mm0
	paddd %mm2,%mm0
	paddd %mm3,%mm0
	movq  %mm0,%mm1
	psrlq $32,%mm1
	paddd %mm1,%mm0
	psrad $13,%mm0
	packssdw %mm0,%mm0
	movd %mm0,%eax
	movw %ax, (%edi)
	leal 32(%esi),%esi
	leal 64(%edx),%edx
	leal 4(%edi),%edi
4:
	subl $64,%esi
	movl $7,%ecx
	.balign 16
5:
	movq  (%edx),%mm0
	movq  64(%edx),%mm4
	pmaddwd (%esi),%mm0
	pmaddwd -32(%esi),%mm4
	movq  8(%edx),%mm1
	movq  72(%edx),%mm5
	pmaddwd 8(%esi),%mm1
	pmaddwd -24(%esi),%mm5
	movq  16(%edx),%mm2
	movq  80(%edx),%mm6
	pmaddwd 16(%esi),%mm2
	pmaddwd -16(%esi),%mm6
	movq  24(%edx),%mm3
	movq  88(%edx),%mm7
	pmaddwd 24(%esi),%mm3
	pmaddwd -8(%esi),%mm7
	paddd %mm1,%mm0
	paddd %mm5,%mm4
	paddd %mm2,%mm0
	paddd %mm6,%mm4
	paddd %mm3,%mm0
	paddd %mm7,%mm4
	movq  %mm0,%mm1
	movq  %mm4,%mm5
	psrlq $32,%mm1
	psrlq $32,%mm5
	paddd %mm0,%mm1
	paddd %mm4,%mm5
	psrad $13,%mm1
	psrad $13,%mm5
	packssdw %mm1,%mm1
	packssdw %mm5,%mm5
	psubd %mm0,%mm0
	psubd %mm4,%mm4
	psubsw %mm1,%mm0
	psubsw %mm5,%mm4
	movq	(%edi), %mm1
	punpckldq %mm4, %mm0
	pand   one_null, %mm1
	pand   null_one, %mm0
	por    %mm0, %mm1
	movq   %mm1,(%edi)
	subl $64,%esi
	addl $128,%edx
	leal 8(%edi),%edi
	decl %ecx
	jnz  5b
	movq  (%edx),%mm0
	pmaddwd (%esi),%mm0
	movq  8(%edx),%mm1
	pmaddwd 8(%esi),%mm1
	movq  16(%edx),%mm2
	pmaddwd 16(%esi),%mm2
	movq  24(%edx),%mm3
	pmaddwd 24(%esi),%mm3
	paddd %mm1,%mm0
	paddd %mm2,%mm0
	paddd %mm3,%mm0
	movq  %mm0,%mm1
	psrlq $32,%mm1
	paddd %mm0,%mm1
	psrad $13,%mm1
	packssdw %mm1,%mm1
	psubd %mm0,%mm0
	psubsw %mm1,%mm0
	movd %mm0,%eax
	movw %ax,(%edi)
	emms


	popl	%ebx
	popl	%esi
	popl	%edi
	addl $4,%esp
	popl	%ebp
	ret



