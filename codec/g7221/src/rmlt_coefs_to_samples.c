/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : rmlt_coefs_to_samples.c
**
**   ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
******************************************************************
*****************************************************************/

/*********************************************************************************
* RMLT_COEFS_TO_SAMPLES		Convert Reversed MLT (Modulated Lapped Transform)
*				Coefficients to Samples
*
*     The "Reversed MLT" is an overlapped block transform which uses even symmetry
* on the left, odd symmetry on the right and a Type IV DCT as the block transform.
* It is thus similar to a MLT which uses odd symmetry on the left, even symmetry
* on the right and a Type IV DST as the block transform.  In fact, it is equivalent
* to reversing the order of the samples, performing an MLT and then negating all
* the even-numbered coefficients.
*
*********************************************************************************/

/************************************************************************************
 Include files                                                           
*************************************************************************************/

#include <stdio.h>
#include <math.h>
#include "defs.h"

/***************************************************************************
 Procedure/Function:  rmlt_samples_to_coefs 

 Syntax:       void rmlt_coefs_to_samples(coefs,
			   out_samples,
			   dct_size)

				float *coefs;
				float *out_samples;
				int dct_size;
               
			inputs:    float *coefs
                       int dct_size
                                                                         
            outputs:   float *out_samples
               
 Description:    Converts the mlt_coefs to samples
				
***************************************************************************/

void rmlt_coefs_to_samples(coefs,
			   out_samples,
			   dct_size)
  float *coefs;
  float *out_samples;
  int dct_size;
{
  extern void dct_type_iv(float *, float *, long);

   static float	old_samples[MAX_DCT_SIZE>>1];
   static float	window[MAX_DCT_SIZE];
   static int	here_before = 0;
   float sum;
   
   int index, vals_left;
   double angle;
   float new_samples[MAX_DCT_SIZE];
   float *new_ptr, *old_ptr;
   float *win_new, *win_old;
   float *out_ptr;
   int half_dct_size;


   half_dct_size = dct_size>>1;

   /*++++++++++++++++++++++++++++++++++++++*/
   /* Set up some data the first time here */
   /*++++++++++++++++++++++++++++++++++++++*/
   
   if (here_before == 0) {
     for (index = 0;    index < dct_size;    index++) {
       angle         = (PI/2.0) * ((double) index + 0.5) / (double)dct_size;

/* This changed when ENCODER_SCALE_FACTOR changed from 20853.0 to 18318.0. */
/*       window[index] = 2.20895 * sin(angle); */
/*       window[index] = (2.20895 * 129.5704536) * sin(angle); */
/*       window[index] = (2.20895 * 129.6) * sin(angle); */
       
	   window[index] = (float)sin(angle);

     }
     for (index = 0;    index < half_dct_size;    index++)
       old_samples[index] = 0.0;
     here_before = 1;
   }
   
   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   /* Perform a Type IV (inverse) DCT on the coefficients */
   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   
  dct_type_iv(coefs, new_samples, dct_size);

   /*++++++++++++++++++++++++++++++++++++++++++++*/
   /* Get the first half of the windowed samples */
   /*++++++++++++++++++++++++++++++++++++++++++++*/
   
   out_ptr = out_samples;
   win_new = window;
   win_old = window + dct_size;
   old_ptr = old_samples;
   new_ptr = new_samples + half_dct_size;
   
   for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
     {
       sum = *win_new++ * *--new_ptr;
       sum += *--win_old * *old_ptr++;
       *out_ptr++ = sum;
     }
       
   /*+++++++++++++++++++++++++++++++++++++++++++++*/
   /* Get the second half of the windowed samples */
   /*+++++++++++++++++++++++++++++++++++++++++++++*/
   
   for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
     {

       sum = *win_new++ *  *new_ptr++;
       sum -= *--win_old *  *--old_ptr;
       *out_ptr++ = sum;

     }

   /*+++++++++++++++++++++++++++++++++++++++++++++++*/
   /* Save the second half of the new samples for   */
   /* next time, when they will be the old samples. */
   /*+++++++++++++++++++++++++++++++++++++++++++++++*/
   
   new_ptr = new_samples + half_dct_size;
   old_ptr = old_samples;
   for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
     *old_ptr++ = *new_ptr++;
}
