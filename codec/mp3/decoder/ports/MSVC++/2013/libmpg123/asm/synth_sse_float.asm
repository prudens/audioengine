



















































































































































































































































































































































































































































































	.section	.rodata



	.balign 32
_scale_sse:
	.long   939524096
	.long   939524096
	.long   939524096
	.long   939524096
	.text
	.balign 16
.globl _INT123_synth_1to1_real_sse_asm
_INT123_synth_1to1_real_sse_asm:
	pushl		%ebp
	movl		%esp, %ebp
	pushl		%ebx
	pushl		%esi
	
	movl		8(%ebp), %ebx
	movl		12(%ebp), %edx
	movl		16(%ebp), %esi
	movl		20(%ebp), %eax
	shll		$2, %eax
	
	leal		64(%ebx), %ebx
	subl		%eax, %ebx

	movl		$4, %ecx
	
	.balign 16
1:
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movups		128(%ebx), %xmm4
	movups		144(%ebx), %xmm5
	movups		160(%ebx), %xmm6
	movups		176(%ebx), %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		64(%edx), %xmm4
	mulps		80(%edx), %xmm5
	mulps		96(%edx), %xmm6
	mulps		112(%edx), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm4, %xmm5
	movaps		%xmm0, %xmm4
	
	leal		256(%ebx), %ebx
	leal		128(%edx), %edx
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movups		128(%ebx), %xmm6
	movups		144(%ebx), %xmm7
	mulps		(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		64(%edx), %xmm6
	mulps		80(%edx), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm7, %xmm6
	movups		160(%ebx), %xmm1
	movups		176(%ebx), %xmm3
	mulps		96(%edx), %xmm1
	mulps		112(%edx), %xmm3
	addps		%xmm2, %xmm0
	addps		%xmm3, %xmm1
	addps		%xmm1, %xmm6
	movaps		%xmm6, %xmm7
	movaps		%xmm0, %xmm6
	
	leal		256(%ebx), %ebx
	leal		128(%edx), %edx
	
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	unpcklps	%xmm5, %xmm4
	unpcklps	%xmm7, %xmm6
	unpckhps	%xmm5, %xmm0
	unpckhps	%xmm7, %xmm1
	movaps		%xmm4, %xmm2
	movaps		%xmm0, %xmm3
	movlhps		%xmm6, %xmm4
	movhlps		%xmm2, %xmm6
	movlhps		%xmm1, %xmm0
	movhlps		%xmm3, %xmm1
	subps		%xmm6, %xmm4
	subps		%xmm1, %xmm0
	addps		%xmm4, %xmm0
	
	movups		(%esi), %xmm1
	movups		16(%esi), %xmm2
	mulps		_scale_sse, %xmm0
	shufps		$0xdd, %xmm2, %xmm1
	movaps		%xmm0, %xmm2
	unpcklps	%xmm1, %xmm0
	unpckhps	%xmm1, %xmm2
	movups		%xmm0, (%esi)
	movups		%xmm2, 16(%esi)
	
	leal		32(%esi), %esi
	decl		%ecx
	jnz			1b
	
	movl		$4, %ecx
	
	.balign 16
1:
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movups		128(%ebx), %xmm4
	movups		144(%ebx), %xmm5
	movups		160(%ebx), %xmm6
	movups		176(%ebx), %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		-64(%edx), %xmm4
	mulps		-48(%edx), %xmm5
	mulps		-32(%edx), %xmm6
	mulps		-16(%edx), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm4, %xmm5
	movaps		%xmm0, %xmm4
	
	leal		256(%ebx), %ebx
	leal		-128(%edx), %edx
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movups		128(%ebx), %xmm6
	movups		144(%ebx), %xmm7
	mulps		(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		-64(%edx), %xmm6
	mulps		-48(%edx), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm7, %xmm6
	movups		160(%ebx), %xmm1
	movups		176(%ebx), %xmm3
	mulps		-32(%edx), %xmm1
	mulps		-16(%edx), %xmm3
	addps		%xmm2, %xmm0
	addps		%xmm3, %xmm1
	addps		%xmm1, %xmm6
	movaps		%xmm6, %xmm7
	movaps		%xmm0, %xmm6
	
	leal		256(%ebx), %ebx
	leal		-128(%edx), %edx
	
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	unpcklps	%xmm5, %xmm4
	unpcklps	%xmm7, %xmm6
	unpckhps	%xmm5, %xmm0
	unpckhps	%xmm7, %xmm1
	movaps		%xmm4, %xmm2
	movaps		%xmm0, %xmm3
	movlhps		%xmm6, %xmm4
	movhlps		%xmm2, %xmm6
	movlhps		%xmm1, %xmm0
	movhlps		%xmm3, %xmm1
	addps		%xmm6, %xmm4
	addps		%xmm1, %xmm0
	addps		%xmm4, %xmm0
	
	movups		(%esi), %xmm1
	movups		16(%esi), %xmm2
	mulps		_scale_sse, %xmm0
	shufps		$0xdd, %xmm2, %xmm1
	movaps		%xmm0, %xmm2
	unpcklps	%xmm1, %xmm0
	unpckhps	%xmm1, %xmm2
	movups		%xmm0, (%esi)
	movups		%xmm2, 16(%esi)
	
	leal		32(%esi), %esi
	decl		%ecx
	jnz			1b
	
	xorl		%eax, %eax
	
	popl		%esi
	popl		%ebx
	movl		%ebp, %esp
	popl		%ebp
	
	ret


