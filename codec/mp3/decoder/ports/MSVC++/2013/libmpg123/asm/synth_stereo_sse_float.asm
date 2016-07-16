























































































































































































































































































































































































































































































	.section	.rodata



	.balign 32
_scale_sse:
	.long   939524096
	.long   939524096
	.long   939524096
	.long   939524096
	.text
	.balign 16
.globl _INT123_synth_1to1_real_s_sse_asm
_INT123_synth_1to1_real_s_sse_asm:
	pushl		%ebp
	movl		%esp, %ebp
	andl		$-16, %esp
	subl		$128, %esp
	pushl		%ebx
	pushl		%esi
	pushl		%edi
	
	movl		8(%ebp), %ebx
	movl		12(%ebp), %edx
	movl		16(%ebp), %esi
	movl		20(%ebp), %edi
	movl		24(%ebp), %eax
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
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*0)(%esp)
	movaps		%xmm4, (12+16*4)(%esp)
	
	leal		128(%ebx), %ebx
	leal		64(%edx), %edx
	leal		64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*1)(%esp)
	movaps		%xmm4, (12+16*5)(%esp)
	
	leal		128(%ebx), %ebx
	leal		64(%edx), %edx
	leal		64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*2)(%esp)
	movaps		%xmm4, (12+16*6)(%esp)
	
	leal		128(%ebx), %ebx
	leal		64(%edx), %edx
	leal		64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, %xmm7
	movaps		%xmm4, (12+16*7)(%esp)
	
	leal		128(%ebx), %ebx
	leal		64(%edx), %edx
	leal		64(%esi), %esi
	
	movaps		(12+16*0)(%esp), %xmm4
	movaps		(12+16*1)(%esp), %xmm5
	movaps		(12+16*2)(%esp), %xmm6
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
	movaps		%xmm0, %xmm2
	
	movaps		(12+16*4)(%esp), %xmm4
	movaps		(12+16*5)(%esp), %xmm5
	movaps		(12+16*6)(%esp), %xmm6
	movaps		(12+16*7)(%esp), %xmm7
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	unpcklps	%xmm5, %xmm4
	unpcklps	%xmm7, %xmm6
	unpckhps	%xmm5, %xmm0
	unpckhps	%xmm7, %xmm1
	movaps		%xmm2, %xmm5
	movaps		%xmm4, %xmm2
	movaps		%xmm0, %xmm3
	movlhps		%xmm6, %xmm4
	movhlps		%xmm2, %xmm6
	movlhps		%xmm1, %xmm0
	movhlps		%xmm3, %xmm1
	subps		%xmm6, %xmm4
	subps		%xmm1, %xmm0
	addps		%xmm4, %xmm0
	
	mulps		_scale_sse, %xmm5
	mulps		_scale_sse, %xmm0
	movaps		%xmm5, %xmm1
	unpcklps	%xmm0, %xmm5
	unpckhps	%xmm0, %xmm1
	movups		%xmm5, (%edi)
	movups		%xmm1, 16(%edi)
	
	leal		32(%edi), %edi
	decl		%ecx
	jnz			1b
	
	movl		$4, %ecx
	
	.balign 16
1:
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*0)(%esp)
	movaps		%xmm4, (12+16*4)(%esp)
	
	leal		128(%ebx), %ebx
	leal		-64(%edx), %edx
	leal		-64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*1)(%esp)
	movaps		%xmm4, (12+16*5)(%esp)
	
	leal		128(%ebx), %ebx
	leal		-64(%edx), %edx
	leal		-64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, (12+16*2)(%esp)
	movaps		%xmm4, (12+16*6)(%esp)
	
	leal		128(%ebx), %ebx
	leal		-64(%edx), %edx
	leal		-64(%esi), %esi
	
	movups		(%ebx), %xmm0
	movups		16(%ebx), %xmm1
	movups		32(%ebx), %xmm2
	movups		48(%ebx), %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm1, %xmm5
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	mulps		0(%edx), %xmm0
	mulps		16(%edx), %xmm1
	mulps		32(%edx), %xmm2
	mulps		48(%edx), %xmm3
	mulps		0(%esi), %xmm4
	mulps		16(%esi), %xmm5
	mulps		32(%esi), %xmm6
	mulps		48(%esi), %xmm7
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	addps		%xmm5, %xmm4
	addps		%xmm7, %xmm6
	addps		%xmm2, %xmm0
	addps		%xmm6, %xmm4
	movaps		%xmm0, %xmm7
	movaps		%xmm4, (12+16*7)(%esp)
	
	leal		128(%ebx), %ebx
	leal		-64(%edx), %edx
	leal		-64(%esi), %esi
	
	movaps		(12+16*0)(%esp), %xmm4
	movaps		(12+16*1)(%esp), %xmm5
	movaps		(12+16*2)(%esp), %xmm6
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
	movaps		%xmm0, %xmm2
	
	movaps		(12+16*4)(%esp), %xmm4
	movaps		(12+16*5)(%esp), %xmm5
	movaps		(12+16*6)(%esp), %xmm6
	movaps		(12+16*7)(%esp), %xmm7
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	unpcklps	%xmm5, %xmm4
	unpcklps	%xmm7, %xmm6
	unpckhps	%xmm5, %xmm0
	unpckhps	%xmm7, %xmm1
	movaps		%xmm2, %xmm5
	movaps		%xmm4, %xmm2
	movaps		%xmm0, %xmm3
	movlhps		%xmm6, %xmm4
	movhlps		%xmm2, %xmm6
	movlhps		%xmm1, %xmm0
	movhlps		%xmm3, %xmm1
	addps		%xmm6, %xmm4
	addps		%xmm1, %xmm0
	addps		%xmm4, %xmm0
	
	mulps		_scale_sse, %xmm5
	mulps		_scale_sse, %xmm0
	movaps		%xmm5, %xmm1
	unpcklps	%xmm0, %xmm5
	unpckhps	%xmm0, %xmm1
	movups		%xmm5, (%edi)
	movups		%xmm1, 16(%edi)
	
	leal		32(%edi), %edi
	decl		%ecx
	jnz			1b
	
	xorl		%eax, %eax
	
	popl		%edi
	popl		%esi
	popl		%ebx
	movl		%ebp, %esp
	popl		%ebp
	
	ret


