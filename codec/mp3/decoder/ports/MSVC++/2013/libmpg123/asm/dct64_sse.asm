















































































































































































































































































































































































































































































	.section	.rodata



	.balign 16
pnpn:
	.long	0
	.long	-2147483648
	.long	0
	.long	-2147483648
	.balign 16
mask:
	.long	-1
	.long	-1
	.long	-1
	.long	0
	
	.text
	.balign 16
.globl INT123_dct64_sse
INT123_dct64_sse:
	pushl		%ebp
	movl		%esp, %ebp
	
	andl		$-16, %esp 
	subl		$128, %esp 
	pushl		%ebx
	
	movl		(8+0*4)(%ebp), %ecx
	movl		(8+1*4)(%ebp), %ebx
	movl		(8+2*4)(%ebp), %eax
	
	movups 	(%eax), %xmm7
	movups 	16(%eax), %xmm6
	movups 	112(%eax), %xmm0
	movups 	96(%eax), %xmm1
	shufps 		$0x1b, %xmm0, %xmm0
	shufps 		$0x1b, %xmm1, %xmm1
	movaps 		%xmm7, %xmm4
	movaps		%xmm6, %xmm5
	addps 		%xmm0, %xmm4
	addps 		%xmm1, %xmm5
	subps 		%xmm0, %xmm7
	subps 		%xmm1, %xmm6
	movaps		%xmm4, (4+0*16)(%esp)
	movaps		%xmm5, (4+1*16)(%esp)
	
	movups 	32(%eax), %xmm2
	movups 	48(%eax), %xmm3
	movups 	80(%eax), %xmm0
	movups 	64(%eax), %xmm1
	shufps 		$0x1b, %xmm0, %xmm0
	shufps 		$0x1b, %xmm1, %xmm1
	movaps 		%xmm2, %xmm5
	movaps		%xmm3, %xmm4
	addps 		%xmm0, %xmm2
	addps 		%xmm1, %xmm3
	subps 		%xmm0, %xmm5
	subps 		%xmm1, %xmm4
	
	mulps		INT123_costab_mmxsse, %xmm7
	mulps		INT123_costab_mmxsse+16, %xmm6
	mulps		INT123_costab_mmxsse+32, %xmm5
	mulps		INT123_costab_mmxsse+48, %xmm4
	
	shufps		$0x1b, %xmm2, %xmm2
	shufps		$0x1b, %xmm3, %xmm3
	shufps		$0x1b, %xmm4, %xmm4
	shufps		$0x1b, %xmm5, %xmm5
	movaps		(4+0*16)(%esp), %xmm0
	movaps		(4+1*16)(%esp), %xmm1
	subps		%xmm3, %xmm0
	subps		%xmm2, %xmm1
	addps		(4+0*16)(%esp), %xmm3
	addps		(4+1*16)(%esp), %xmm2
	movaps		%xmm3, (4+0*16)(%esp)
	movaps		%xmm2, (4+1*16)(%esp)
	movaps		%xmm6, %xmm2
	movaps		%xmm7, %xmm3
	subps		%xmm5, %xmm6
	subps		%xmm4, %xmm7
	addps		%xmm3, %xmm4
	addps		%xmm2, %xmm5
	mulps		INT123_costab_mmxsse+64, %xmm0
	mulps		INT123_costab_mmxsse+80, %xmm1
	mulps		INT123_costab_mmxsse+80, %xmm6
	mulps		INT123_costab_mmxsse+64, %xmm7
	
	movaps		(4+0*16)(%esp), %xmm2
	movaps		(4+1*16)(%esp), %xmm3
	shufps		$0x1b, %xmm3, %xmm3
	shufps		$0x1b, %xmm5, %xmm5
	shufps		$0x1b, %xmm1, %xmm1
	shufps		$0x1b, %xmm6, %xmm6
	movaps		%xmm0, (4+1*16)(%esp)
	subps		%xmm3, %xmm2
	subps		%xmm1, %xmm0
	addps		(4+0*16)(%esp), %xmm3
	addps		(4+1*16)(%esp), %xmm1
	movaps		%xmm3, (4+0*16)(%esp)
	movaps		%xmm1, (4+2*16)(%esp)
	movaps		%xmm5, %xmm1
	movaps		%xmm4, %xmm5
	movaps		%xmm7, %xmm3
	subps		%xmm1, %xmm5
	subps		%xmm6, %xmm7
	addps		%xmm1, %xmm4
	addps		%xmm3, %xmm6
	mulps		INT123_costab_mmxsse+96, %xmm2
	mulps		INT123_costab_mmxsse+96, %xmm0
	mulps		INT123_costab_mmxsse+96, %xmm5
	mulps		INT123_costab_mmxsse+96, %xmm7
	movaps		%xmm2, (4+1*16)(%esp)
	movaps		%xmm0, (4+3*16)(%esp)
	
	movaps		%xmm4, %xmm2
	movaps		%xmm5, %xmm3
	shufps		$0x44, %xmm6, %xmm2
	shufps		$0xbb, %xmm7, %xmm5
	shufps		$0xbb, %xmm6, %xmm4
	shufps		$0x44, %xmm7, %xmm3
	movaps		%xmm2, %xmm6
	movaps		%xmm3, %xmm7
	subps		%xmm4, %xmm2
	subps		%xmm5, %xmm3
	addps		%xmm6, %xmm4
	addps		%xmm7, %xmm5
	movaps		INT123_costab_mmxsse+112, %xmm0
	movlhps		%xmm0, %xmm0
	mulps		%xmm0, %xmm2
	mulps		%xmm0, %xmm3
	movaps		%xmm0, (4+4*16)(%esp)
	movaps		%xmm4, %xmm6
	movaps		%xmm5, %xmm7
	shufps		$0x14, %xmm2, %xmm4
	shufps		$0xbe, %xmm2, %xmm6
	shufps		$0x14, %xmm3, %xmm5
	shufps		$0xbe, %xmm3, %xmm7
	movaps		%xmm5, (4+5*16)(%esp)
	movaps		%xmm7, (4+7*16)(%esp)
	
	movaps		(4+0*16)(%esp), %xmm0
	movaps		(4+1*16)(%esp), %xmm1
	movaps		%xmm0, %xmm2
	movaps		%xmm1, %xmm3
	shufps		$0x44, (4+2*16)(%esp), %xmm2
	shufps		$0xbb, (4+3*16)(%esp), %xmm1
	shufps		$0xbb, (4+2*16)(%esp), %xmm0
	shufps		$0x44, (4+3*16)(%esp), %xmm3
	movaps		%xmm2, %xmm5
	movaps		%xmm3, %xmm7
	subps		%xmm0, %xmm2
	subps		%xmm1, %xmm3
	addps		%xmm5, %xmm0
	addps		%xmm7, %xmm1
	mulps		(4+4*16)(%esp), %xmm2
	mulps		(4+4*16)(%esp), %xmm3
	movaps		%xmm0, %xmm5
	movaps		%xmm1, %xmm7
	shufps		$0x14, %xmm2, %xmm0
	shufps		$0xbe, %xmm2, %xmm5
	shufps		$0x14, %xmm3, %xmm1
	shufps		$0xbe, %xmm3, %xmm7
	
	movaps		%xmm0, (4+0*16)(%esp)
	movaps		%xmm1, (4+1*16)(%esp)
	movaps		%xmm5, (4+2*16)(%esp)
	movaps		%xmm7, (4+3*16)(%esp)
	
	movss		INT123_costab_mmxsse+120, %xmm5
	shufps		$0x00, %xmm5, %xmm5
	xorps		pnpn, %xmm5
	
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	unpcklps	(4+5*16)(%esp), %xmm4
	unpckhps	(4+5*16)(%esp), %xmm0
	unpcklps	(4+7*16)(%esp), %xmm6
	unpckhps	(4+7*16)(%esp), %xmm1
	movaps		%xmm4, %xmm2
	movaps		%xmm6, %xmm3
	unpcklps	%xmm0, %xmm4
	unpckhps	%xmm0, %xmm2
	unpcklps	%xmm1, %xmm6
	unpckhps	%xmm1, %xmm3
	movaps		%xmm4, %xmm0
	movaps		%xmm6, %xmm1
	subps		%xmm2, %xmm0
	subps		%xmm3, %xmm1
	addps		%xmm2, %xmm4
	addps		%xmm3, %xmm6
	mulps		%xmm5, %xmm0
	mulps		%xmm5, %xmm1
	movaps		%xmm5, (4+5*16)(%esp)
	movaps		%xmm4, %xmm5
	movaps		%xmm6, %xmm7
	unpcklps	%xmm0, %xmm4
	unpckhps	%xmm0, %xmm5
	unpcklps	%xmm1, %xmm6
	unpckhps	%xmm1, %xmm7
	
	movaps		(4+0*16)(%esp), %xmm0
	movaps		(4+2*16)(%esp), %xmm2
	movaps		%xmm4, (4+4*16)(%esp)
	movaps		%xmm6, (4+6*16)(%esp)
	
	movaps		%xmm0, %xmm4
	movaps		%xmm2, %xmm6
	unpcklps	(4+1*16)(%esp), %xmm0
	unpckhps	(4+1*16)(%esp), %xmm4
	unpcklps	(4+3*16)(%esp), %xmm2
	unpckhps	(4+3*16)(%esp), %xmm6
	movaps		%xmm0, %xmm1
	movaps		%xmm2, %xmm3
	unpcklps	%xmm4, %xmm0
	unpckhps	%xmm4, %xmm1
	unpcklps	%xmm6, %xmm2
	unpckhps	%xmm6, %xmm3
	movaps		%xmm0, %xmm4
	movaps		%xmm2, %xmm6
	subps		%xmm1, %xmm4
	subps		%xmm3, %xmm6
	addps		%xmm1, %xmm0
	addps		%xmm3, %xmm2
	mulps		(4+5*16)(%esp), %xmm4
	mulps		(4+5*16)(%esp), %xmm6
	movaps		%xmm0, %xmm1
	movaps		%xmm2, %xmm3
	unpcklps	%xmm4, %xmm0
	unpckhps	%xmm4, %xmm1
	unpcklps	%xmm6, %xmm2
	unpckhps	%xmm6, %xmm3
	
	movaps		%xmm0, (4+0*16)(%esp)
	movaps		%xmm1, (4+1*16)(%esp)
	movaps		%xmm2, (4+2*16)(%esp)
	movaps		%xmm3, (4+3*16)(%esp)
	movaps		%xmm5, (4+5*16)(%esp)
	movaps		%xmm7, (4+7*16)(%esp)
	
	movss		(4+12)(%esp), %xmm0
	movss		(4+28)(%esp), %xmm1
	movss		(4+44)(%esp), %xmm2
	movss		(4+60)(%esp), %xmm3
	addss		(4+8)(%esp), %xmm0
	addss		(4+24)(%esp), %xmm1
	addss		(4+40)(%esp), %xmm2
	addss		(4+56)(%esp), %xmm3
	movss		%xmm0, (4+8)(%esp)
	movss		%xmm1, (4+24)(%esp)
	movss		%xmm2, (4+40)(%esp)
	movss		%xmm3, (4+56)(%esp)
	movss		(4+76)(%esp), %xmm0
	movss		(4+92)(%esp), %xmm1
	movss		(4+108)(%esp), %xmm2
	movss		(4+124)(%esp), %xmm3
	addss		(4+72)(%esp), %xmm0
	addss		(4+88)(%esp), %xmm1
	addss		(4+104)(%esp), %xmm2
	addss		(4+120)(%esp), %xmm3
	movss		%xmm0, (4+72)(%esp)
	movss		%xmm1, (4+88)(%esp)
	movss		%xmm2, (4+104)(%esp)
	movss		%xmm3, (4+120)(%esp)
	
	movaps		(4+16)(%esp), %xmm1
	movaps		(4+48)(%esp), %xmm3
	movaps		(4+80)(%esp), %xmm5
	movaps		(4+112)(%esp), %xmm7
	movaps		%xmm1, %xmm0
	movaps		%xmm3, %xmm2
	movaps		%xmm5, %xmm4
	movaps		%xmm7, %xmm6
	shufps		$0x1e, %xmm0, %xmm0
	shufps		$0x1e, %xmm2, %xmm2
	shufps		$0x1e, %xmm4, %xmm4
	shufps		$0x1e, %xmm6, %xmm6
	andps		mask, %xmm0
	andps		mask, %xmm2
	andps		mask, %xmm4
	andps		mask, %xmm6
	addps		%xmm0, %xmm1
	addps		%xmm2, %xmm3
	addps		%xmm4, %xmm5
	addps		%xmm6, %xmm7
	
	movaps		(4+32)(%esp), %xmm2
	movaps		(4+96)(%esp), %xmm6
	movaps		%xmm2, %xmm0
	movaps		%xmm6, %xmm4
	shufps		$0x1e, %xmm0, %xmm0
	shufps		$0x1e, %xmm4, %xmm4
	andps		mask, %xmm0
	andps		mask, %xmm4
	addps		%xmm3, %xmm2
	addps		%xmm0, %xmm3
	addps		%xmm7, %xmm6
	addps		%xmm4, %xmm7
	
	movaps		(4+0)(%esp), %xmm0
	movaps		(4+64)(%esp), %xmm4
	
	cvtps2pi	%xmm0, %mm0
	cvtps2pi	%xmm1, %mm1
	movhlps		%xmm0, %xmm0
	movhlps		%xmm1, %xmm1
	cvtps2pi	%xmm0, %mm2
	cvtps2pi	%xmm1, %mm3
	packssdw	%mm2, %mm0
	packssdw	%mm3, %mm1
	
	cvtps2pi	%xmm2, %mm2
	cvtps2pi	%xmm3, %mm3
	movhlps		%xmm2, %xmm2
	movhlps		%xmm3, %xmm3
	cvtps2pi	%xmm2, %mm4
	cvtps2pi	%xmm3, %mm5
	packssdw	%mm4, %mm2
	packssdw	%mm5, %mm3
	
	movd		%mm0, %eax
	movd		%mm1, %edx
	movw		%ax, 512(%ecx)
	movw		%dx, 384(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, (%ecx)
	movw		%ax, (%ebx)
	movw		%dx, 128(%ebx)
	
	movd		%mm2, %eax
	movd		%mm3, %edx
	movw		%ax, 448(%ecx)
	movw		%dx, 320(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 64(%ebx)
	movw		%dx, 192(%ebx)
	
	psrlq		$32, %mm0
	psrlq		$32, %mm1
	movd		%mm0, %eax
	movd		%mm1, %edx
	movw		%ax, 256(%ecx)
	movw		%dx, 128(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 256(%ebx)
	movw		%dx, 384(%ebx)
	
	psrlq		$32, %mm2
	psrlq		$32, %mm3
	movd		%mm2, %eax
	movd		%mm3, %edx
	movw		%ax, 192(%ecx)
	movw		%dx, 64(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 320(%ebx)
	movw		%dx, 448(%ebx)
	
	movaps		%xmm4, %xmm0
	shufps		$0x1e, %xmm0, %xmm0
	movaps		%xmm5, %xmm1
	andps		mask, %xmm0
	
	addps		%xmm6, %xmm4
	addps		%xmm7, %xmm5
	addps		%xmm1, %xmm6
	addps		%xmm0, %xmm7
	
	cvtps2pi	%xmm4, %mm0
	cvtps2pi	%xmm5, %mm1
	movhlps		%xmm4, %xmm4
	movhlps		%xmm5, %xmm5
	cvtps2pi	%xmm4, %mm2
	cvtps2pi	%xmm5, %mm3
	packssdw	%mm2, %mm0
	packssdw	%mm3, %mm1
	
	cvtps2pi	%xmm6, %mm2
	cvtps2pi	%xmm7, %mm3
	movhlps		%xmm6, %xmm6
	movhlps		%xmm7, %xmm7
	cvtps2pi	%xmm6, %mm4
	cvtps2pi	%xmm7, %mm5
	packssdw	%mm4, %mm2
	packssdw	%mm5, %mm3
	
	movd		%mm0, %eax
	movd		%mm2, %edx
	movw		%ax, 480(%ecx)
	movw		%dx, 416(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 32(%ebx)
	movw		%dx, 96(%ebx)
	
	psrlq		$32, %mm0
	psrlq		$32, %mm2
	movd		%mm0, %eax
	movd		%mm2, %edx
	movw		%ax, 224(%ecx)
	movw		%dx, 160(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 288(%ebx)
	movw		%dx, 352(%ebx)
	
	movd		%mm1, %eax
	movd		%mm3, %edx
	movw		%ax, 352(%ecx)
	movw		%dx, 288(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 160(%ebx)
	movw		%dx, 224(%ebx)
	
	psrlq		$32, %mm1
	psrlq		$32, %mm3
	movd		%mm1, %eax
	movd		%mm3, %edx
	movw		%ax, 96(%ecx)
	movw		%dx, 32(%ecx)
	shrl		$16, %eax
	shrl		$16, %edx
	movw		%ax, 416(%ebx)
	movw		%dx, 480(%ebx)
	
	popl		%ebx
	movl		%ebp, %esp
	popl		%ebp
	ret


