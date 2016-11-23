





















































































































































































































































































































































































































































































	.section	.rodata



	.balign 32
scale_s32:
	.long   1199570944 
	.long   1199570944
	.long   1199570944
	.long   1199570944
	.balign 16
maxmin_s32:
	.long   1191182335 
	.long   1191182335
	.long   1191182335
	.long   1191182335
	.long   -956301312 
	.long   -956301312
	.long   -956301312
	.long   -956301312
	.text
	.balign 16
.globl INT123_synth_1to1_s32_sse_asm
INT123_synth_1to1_s32_sse_asm:
	pushl		%ebp
	movl		%esp, %ebp
	pushl		%ebx
	pushl		%esi
	
	pxor		%mm7, %mm7
	
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
	
	movaps		%xmm0, %xmm1
	movaps		%xmm0, %xmm2
	mulps		scale_s32, %xmm0
	cmpnleps	maxmin_s32, %xmm1
	cmpltps		maxmin_s32+16, %xmm2
	cvtps2pi	%xmm0, %mm0
	movhlps		%xmm0, %xmm0
	cvtps2pi	%xmm0, %mm1
	cvtps2pi	%xmm1, %mm2
	movhlps		%xmm1, %xmm1
	cvtps2pi	%xmm1, %mm3
	psrad		$31, %mm2
	psrad		$31, %mm3
	pxor		%mm2, %mm0
	pxor		%mm3, %mm1
	movd		%mm0, (%esi)
	psrlq		$32, %mm0
	movd		%mm0, 8(%esi)
	movd		%mm1, 16(%esi)
	psrlq		$32, %mm1
	movd		%mm1, 24(%esi)
	
	cvtps2pi	%xmm2, %mm0
	movhlps		%xmm2, %xmm2
	cvtps2pi	%xmm2, %mm1
	packssdw	%mm3, %mm2
	packssdw	%mm1, %mm0
	psrlw		$15, %mm2
	psrlw		$15, %mm0
	paddw		%mm2, %mm0
	paddw		%mm0, %mm7
	
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
	
	movaps		%xmm0, %xmm1
	movaps		%xmm0, %xmm2
	mulps		scale_s32, %xmm0
	cmpnleps	maxmin_s32, %xmm1
	cmpltps		maxmin_s32+16, %xmm2
	cvtps2pi	%xmm0, %mm0
	movhlps		%xmm0, %xmm0
	cvtps2pi	%xmm0, %mm1
	cvtps2pi	%xmm1, %mm2
	movhlps		%xmm1, %xmm1
	cvtps2pi	%xmm1, %mm3
	psrad		$31, %mm2
	psrad		$31, %mm3
	pxor		%mm2, %mm0
	pxor		%mm3, %mm1
	movd		%mm0, (%esi)
	psrlq		$32, %mm0
	movd		%mm0, 8(%esi)
	movd		%mm1, 16(%esi)
	psrlq		$32, %mm1
	movd		%mm1, 24(%esi)
	
	cvtps2pi	%xmm2, %mm0
	movhlps		%xmm2, %xmm2
	cvtps2pi	%xmm2, %mm1
	packssdw	%mm3, %mm2
	packssdw	%mm1, %mm0
	psrlw		$15, %mm2
	psrlw		$15, %mm0
	paddw		%mm2, %mm0
	paddw		%mm0, %mm7
	
	leal		32(%esi), %esi
	decl		%ecx
	jnz			1b
	
	pshufw		$0xee, %mm7, %mm0
	paddw		%mm7, %mm0
	pshufw		$0x55, %mm0, %mm1
	paddw		%mm1, %mm0
	movd		%mm0, %eax
	andl		$0xffff, %eax
	
	popl		%esi
	popl		%ebx
	movl		%ebp, %esp
	popl		%ebp
	
	emms
	
	ret


