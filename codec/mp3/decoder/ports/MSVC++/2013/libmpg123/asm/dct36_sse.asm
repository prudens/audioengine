


















































































































































































































































































































































































































































































	

	.section	.rodata



	.balign 16
dct36_sse_COS9:
	.long 0x3f5db3d7
	.long 0x3f5db3d7
	.long 0x3f000000
	.long 0x3f000000
	.long 0x3f7c1c5c
	.long 0x3f7c1c5c
	.long 0x3f708fb2
	.long 0x3f708fb2
	.long 0x3f248dbb
	.long 0x3f248dbb
	.long 0x3e31d0d4
	.long 0x3e31d0d4
	.long 0x3eaf1d44
	.long 0x3eaf1d44
	.long 0x3f441b7d
	.long 0x3f441b7d
	.balign 16
dct36_sse_tfcos36:
	.long 0x3f007d2b
	.long 0x3f0483ee
	.long 0x3f0d3b7d
	.long 0x3f1c4257
	.long 0x40b79454
	.long 0x3ff746ea
	.long 0x3f976fd9
	.long 0x3f5f2944
	.long 0x3f3504f3
	.balign 16
dct36_sse_mask:
	.long 0,0xffffffff,0,0xffffffff
	.balign 16
dct36_sse_sign:
	.long 0x80000000,0x80000000,0x80000000,0x80000000
	.text
	.balign 16
	.globl _INT123_dct36_sse
_INT123_dct36_sse:
	push		%ebp
	mov			%esp, %ebp
	and			$-16, %esp
	sub			$80, %esp
	push		%ebx
	push		%esi
	push		%edi
	call		1f
1:
	pop			%ebx
	lea			dct36_sse_COS9-1b(%ebx), %eax
	lea			dct36_sse_tfcos36-1b(%ebx), %edx
	lea			12(%esp), %esi
	movl		8(%ebp), %edi
	
	xorps		%xmm0, %xmm0
	xorps		%xmm5, %xmm5
	movlps		64(%edi), %xmm5
	movups		48(%edi), %xmm4
	movups		32(%edi), %xmm3
	movups		16(%edi), %xmm2
	movups		(%edi), %xmm1
	movaps		%xmm5, %xmm6
	shufps		$0xe1, %xmm6, %xmm6
	movaps		%xmm4, %xmm7
	shufps		$0x93, %xmm7, %xmm7
	movss		%xmm7, %xmm6
	addps		%xmm6, %xmm5
	movaps		%xmm3, %xmm6
	shufps		$0x93, %xmm6, %xmm6
	movss		%xmm6, %xmm7
	addps		%xmm7, %xmm4
	movaps		%xmm2, %xmm7
	shufps		$0x93, %xmm7, %xmm7
	movss		%xmm7, %xmm6
	addps		%xmm6, %xmm3
	movaps		%xmm1, %xmm6
	shufps		$0x93, %xmm6, %xmm6
	movss		%xmm6, %xmm7
	addps		%xmm7, %xmm2
	movss		%xmm0, %xmm6
	addps		%xmm6, %xmm1
	
	movaps		dct36_sse_mask-1b(%ebx), %xmm0
	movaps		%xmm4, %xmm6
	shufps		$0x4e, %xmm5, %xmm4
	movaps		%xmm3, %xmm7
	shufps		$0x4e, %xmm6, %xmm3
	andps		%xmm0, %xmm6
	addps		%xmm6, %xmm4
	movaps		%xmm2, %xmm6
	shufps		$0x4e, %xmm7, %xmm2
	andps		%xmm0, %xmm7
	addps		%xmm7, %xmm3
	movaps		%xmm1, %xmm7
	shufps		$0x4e, %xmm6, %xmm1
	andps		%xmm0, %xmm6
	addps		%xmm6, %xmm2
	movaps		%xmm7, %xmm6
	andps		%xmm0, %xmm7
	xorps		%xmm0, %xmm0
	addps		%xmm7, %xmm1
	movlhps		%xmm6, %xmm0









	movaps		%xmm2, %xmm5
	shufps		$0xe4, %xmm3, %xmm5
	shufps		$0xe4, %xmm4, %xmm3
	shufps		$0xe4, %xmm2, %xmm4
	movaps		%xmm5, %xmm2







	mulps		(%eax), %xmm5
	addps		%xmm0, %xmm5
	
	movaps		%xmm0, (%esi)
	movaps		%xmm2, 16(%esi)






	movaps		%xmm1, %xmm6
	subps		%xmm3, %xmm6
	subps		%xmm4, %xmm6
	xorps		%xmm7, %xmm7
	shufps		$0xe0, %xmm2, %xmm7
	mulps		(%eax), %xmm6
	subps		%xmm7, %xmm0
	addps		%xmm0, %xmm6
	movaps		%xmm6, 48(%esi)
	
	movaps		16(%eax), %xmm2

	movaps		%xmm1, %xmm0
	movaps		%xmm3, %xmm6
	movaps		%xmm4, %xmm7
	mulps		%xmm2, %xmm0
	mulps		32(%eax), %xmm6
	mulps		48(%eax), %xmm7
	addps		%xmm5, %xmm0
	addps		%xmm7, %xmm6
	addps		%xmm6, %xmm0
	movaps		%xmm0, 32(%esi)
	
	movaps		%xmm1, %xmm0
	movaps		%xmm3, %xmm6
	movaps		%xmm4, %xmm7
	mulps		32(%eax), %xmm0
	mulps		48(%eax), %xmm6
	mulps		%xmm2, %xmm7
	subps		%xmm5, %xmm0
	subps		%xmm6, %xmm7
	addps		%xmm7, %xmm0
	movaps		%xmm0, 64(%esi)
	
	movaps		%xmm1, %xmm6
	movaps		%xmm4, %xmm7
	mulps		48(%eax), %xmm6
	mulps		%xmm3, %xmm2
	mulps		32(%eax), %xmm7
	subps		%xmm5, %xmm6
	subps		%xmm7, %xmm2
	addps		%xmm2, %xmm6
	
	movaps		(%esi), %xmm0
	movss		32(%edx), %xmm5
	subps		%xmm1, %xmm0
	subps		16(%esi), %xmm4
	addps		%xmm3, %xmm0
	addps		%xmm4, %xmm0
	shufps		$0xaf, %xmm0, %xmm0
	mulss		%xmm5, %xmm0
	movaps		%xmm0, (%esi)
	
	movaps		32(%esi), %xmm0
	movaps		48(%esi), %xmm1
	movaps		64(%esi), %xmm2








	movaps		%xmm0, %xmm3
	unpcklps	%xmm1, %xmm0
	unpckhps	%xmm1, %xmm3
	movaps		%xmm2, %xmm5
	unpcklps	%xmm6, %xmm2
	unpckhps	%xmm6, %xmm5
	xorps		dct36_sse_sign-1b(%ebx), %xmm5








	movaps		%xmm0, %xmm1
	movlhps		%xmm2, %xmm0
	movhlps		%xmm1, %xmm2
	movaps		%xmm3, %xmm4
	movlhps		%xmm5, %xmm3
	movhlps		%xmm4, %xmm5








	movaps		(%edx), %xmm6
	movaps		16(%edx), %xmm7
	movaps		%xmm5, %xmm1
	addps		%xmm2, %xmm5
	subps		%xmm2, %xmm1
	movaps		%xmm3, %xmm2
	addps		%xmm0, %xmm3
	subps		%xmm0, %xmm2
	mulps		%xmm6, %xmm5
	mulps		%xmm1, %xmm7
	
	movaps		%xmm2, 16(%esi)









	movl		12(%ebp), %edi
	movl		16(%ebp), %edx
	movl		20(%ebp), %ecx
	movl		24(%ebp), %eax

	movaps		%xmm3, %xmm0
	movaps		%xmm5, %xmm1
	movups		108(%ecx), %xmm2
	movups		92(%ecx), %xmm3
	shufps		$0x1b, %xmm3, %xmm3
	movups		36(%ecx), %xmm4
	movups		20(%ecx), %xmm5
	shufps		$0x1b, %xmm5, %xmm5
	movaps		%xmm0, %xmm6
	addps		%xmm1, %xmm0
	subps		%xmm1, %xmm6
	mulps		%xmm0, %xmm2
	mulps		%xmm3, %xmm0
	mulps		%xmm6, %xmm4
	mulps		%xmm5, %xmm6
	movups		36(%edi), %xmm1
	movups		20(%edi), %xmm3
	shufps		$0x1b, %xmm6, %xmm6
	addps		%xmm4, %xmm1
	addps		%xmm6, %xmm3
	shufps		$0x1b, %xmm0, %xmm0
	movups		%xmm2, 36(%edx)
	movups		%xmm0, 20(%edx)
	movss		%xmm1, 32*36(%eax)
	movss		%xmm3, 32*20(%eax)
	movhlps		%xmm1, %xmm2
	movhlps		%xmm3, %xmm4
	movss		%xmm2, 32*44(%eax)
	movss		%xmm4, 32*28(%eax)
	shufps		$0xb1, %xmm1, %xmm1
	shufps		$0xb1, %xmm3, %xmm3
	movss		%xmm1, 32*40(%eax)
	movss		%xmm3, 32*24(%eax)
	movhlps		%xmm1, %xmm2
	movhlps		%xmm3, %xmm4
	movss		%xmm2, 32*48(%eax)
	movss		%xmm4, 32*32(%eax)
	
	movss		8(%esi), %xmm0
	movss		(%esi), %xmm1
	movss		124(%ecx), %xmm2
	movss		88(%ecx), %xmm3
	movss		52(%ecx), %xmm4
	movss		16(%ecx), %xmm5
	movss		%xmm0, %xmm6
	addss		%xmm1, %xmm0
	subss		%xmm1, %xmm6
	mulss		%xmm0, %xmm2
	mulss		%xmm3, %xmm0
	mulss		%xmm6, %xmm4
	mulss		%xmm5, %xmm6
	addss		52(%edi), %xmm4
	addss		16(%edi), %xmm6
	movss		%xmm2, 52(%edx)
	movss		%xmm0, 16(%edx)
	movss		%xmm4, 32*52(%eax)
	movss		%xmm6, 32*16(%eax)
	
	movaps		16(%esi), %xmm0
	movaps		%xmm7, %xmm1
	movups		128(%ecx), %xmm2
	movups		72(%ecx), %xmm3
	shufps		$0x1b, %xmm2, %xmm2
	movlps		56(%ecx), %xmm4
	movhps		64(%ecx), %xmm4
	movups		(%ecx), %xmm5
	shufps		$0x1b, %xmm4, %xmm4
	movaps		%xmm0, %xmm6
	addps		%xmm1, %xmm0
	subps		%xmm1, %xmm6
	mulps		%xmm0, %xmm2
	mulps		%xmm3, %xmm0
	mulps		%xmm6, %xmm4
	mulps		%xmm5, %xmm6
	movlps		56(%edi), %xmm1
	movhps		64(%edi), %xmm1
	movups		(%edi), %xmm3
	shufps		$0x1b, %xmm4, %xmm4
	addps		%xmm6, %xmm3
	addps		%xmm4, %xmm1
	shufps		$0x1b, %xmm2, %xmm2
	movups		%xmm0, (%edx)
	movlps		%xmm2, 56(%edx)
	movhps		%xmm2, 64(%edx)
	movss		%xmm1, 32*56(%eax)
	movss		%xmm3, (%eax)
	movhlps		%xmm1, %xmm2
	movhlps		%xmm3, %xmm4
	movss		%xmm2, 32*64(%eax)
	movss		%xmm4, 32*8(%eax)
	shufps		$0xb1, %xmm1, %xmm1
	shufps		$0xb1, %xmm3, %xmm3
	movss		%xmm1, 32*60(%eax)
	movss		%xmm3, 32*4(%eax)
	movhlps		%xmm1, %xmm2
	movhlps		%xmm3, %xmm4
	movss		%xmm2, 32*68(%eax)
	movss		%xmm4, 32*12(%eax)
	
	pop			%edi
	pop			%esi
	pop			%ebx
	mov			%ebp, %esp
	pop			%ebp
	
	ret


