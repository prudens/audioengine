/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : defs.h
**
**   ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
******************************************************************
*****************************************************************/

#define  PI  3.141592653589793238462

#define MAX_DCT_SIZE 640

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#define MAX_NUM_REGIONS 28 

#define NUM_REGIONS 14

#define REGION_POWER_TABLE_SIZE 64
#define REGION_POWER_TABLE_NUM_NEGATIVES 24

#define MAX_NUM_RATE_CONTROL_BITS 5
#define MAX_NUM_RATE_CONTROL_POSSIBILITIES 32

#define ENCODER_SCALE_FACTOR 18318.0

/* The MLT output is incorrectly scaled by the factor
   product of ENCODER_SCALE_FACTOR and sqrt(160.)
   This is now (9/30/96) 1.0/2^(4.5) or 1/22.627.
   In the current implementation this product
   must be an integer power of sqrt(2). The
   integer power is ESF_ADJUSTMENT_TO_RMS_INDEX.
   The -2 is to conform with the range defined in the spec. */

/*
** This value has been changed for fixed point interop 
*/
#define ESF_ADJUSTMENT_TO_RMS_INDEX (9-2) // changed from (-2)

#define INTERMEDIATE_FILES_FLAG 0

/* Max bit rate is now 48000 bits/sec. */

#define MAX_BITS_PER_FRAME 960



