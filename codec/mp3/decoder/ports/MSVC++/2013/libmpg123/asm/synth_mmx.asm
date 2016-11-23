











































































































































































































































































































































































































































































.text

.globl INT123_synth_1to1_MMX

INT123_synth_1to1_MMX:
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %ebx

        movl 24(%esp),%ecx
        movl 28(%esp),%edi
        movl $15,%ebx
        movl 36(%esp),%edx
        leal (%edi,%ecx,2),%edi
	decl %ecx
        movl 32(%esp),%esi
        movl (%edx),%eax
        jecxz 1f
        decl %eax
        andl %ebx,%eax
        leal 1088(%esi),%esi
        movl %eax,(%edx)
1:
        leal (%esi,%eax,2),%edx
        movl %eax,%ebp
        incl %eax
        pushl 20(%esp)
        andl %ebx,%eax
        leal 544(%esi,%eax,2),%ecx
        incl %ebx
	testl $1, %eax
	jnz 2f
        xchgl %edx,%ecx
	incl %ebp
        leal 544(%esi),%esi
2:
        pushl %edx
        pushl %ecx
        call INT123_dct64_MMX
        addl $12,%esp

	leal 1(%ebx), %ecx
        subl %ebp,%ebx
	pushl %eax
	movl 44(%esp),%eax 
	leal (%eax,%ebx,2), %edx
	popl %eax
3:
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
        loop 3b


        subl $64,%esi
        movl $15,%ecx
4:
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

        subl $32,%esi
        addl $64,%edx
        leal 4(%edi),%edi
        loop 4b
	emms
        popl %ebx
        popl %esi
        popl %edi
        popl %ebp
        ret


