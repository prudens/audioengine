/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : samples_to_rmlt_coefs.c
**
**   ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
******************************************************************
*****************************************************************/

/************************************************************************************
 Include files                                                           
*************************************************************************************/
#include <stdio.h>
#include <math.h>
#include "defs.h"

#define ABS(a)  (a>0?a:-a)

/***************************************************************************
 Procedure/Function:  samples_to_rmlt_coefs 

 Syntax:     void samples_to_rmlt_coefs(new_samples,
										coefs,
										dct_size)
				float *new_samples;
				float *coefs;
				int dct_size;
      
 
 Description: samples_to_rmlt_coefs 	Convert Samples to Reversed MLT (Modulated Lapped
										Transform) Coefficients

     The "Reversed MLT" is an overlapped block transform which uses even symmetry
 on the left, odd symmetry on the right and a Type IV DCT as the block transform.
 It is thus similar to a MLT which uses odd symmetry on the left, even symmetry
 on the right and a Type IV DST as the block transform.  In fact, it is equivalent
 to reversing the order of the samples, performing an MLT and then negating all
 the even-numbered coefficients.
 

 Design Notes:
				
***************************************************************************/

void samples_to_rmlt_coefs(new_samples,
			   coefs,
			   dct_size)
  float *new_samples;
  float *coefs;
  int dct_size;

  {
  extern void dct_type_iv(float *, float *, long);
  static float	old_samples[MAX_DCT_SIZE];
  static float	window[MAX_DCT_SIZE];
  static int	here_before = 0;

  int index, vals_left;
  float sum;
  double angle;
  float	windowed_data[MAX_DCT_SIZE];
  float	*new_ptr, *old_ptr, *sam_low, *sam_high;
  float	*win_low, *win_high;
  float	*dst_ptr;
  int half_dct_size;
   
  half_dct_size = dct_size>>1;
   
   /*++++++++++++++++++++++++++++++++++++++*/
   /* Set up some data the first time here */
   /*++++++++++++++++++++++++++++++++++++++*/
   
   if (here_before == 0) {
      for (index = 0;    index < dct_size;    index++) {
	 angle         = (PI/2.0) * ((double) index + 0.5) / (double)dct_size;

	 window[index] = (float)sin(angle);

      }
      for (index = 0;    index < dct_size;    index++)
	 old_samples[index] = 0.0;
      here_before = 1;
   }
   
   /*++++++++++++++++++++++++++++++++++++++++++++*/
   /* Get the first half of the windowed samples */
   /*++++++++++++++++++++++++++++++++++++++++++++*/
   
   dst_ptr  = windowed_data;
   win_high = window + half_dct_size;
   win_low  = win_high;
   
   sam_high = old_samples + half_dct_size;
   sam_low  = sam_high;
   for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
       {

       sum = *--win_low *  *--sam_low;
       sum += *win_high++ * *sam_high++;
      *dst_ptr++ = sum;

       }
   
   /*+++++++++++++++++++++++++++++++++++++++++++++*/
   /* Get the second half of the windowed samples */
   /*+++++++++++++++++++++++++++++++++++++++++++++*/
   
   sam_low  = new_samples;
   sam_high = new_samples + dct_size;
   for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
       {

       sum = *--win_high * *sam_low++;
       sum -= *win_low++ * *--sam_high;
      *dst_ptr++ = sum;

       }
      
   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   /* Save the new samples for next time, when they will be the old samples */
   /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   
   new_ptr = new_samples;
   old_ptr = old_samples;
   for (vals_left = dct_size;    vals_left > 0;    vals_left--)
      *old_ptr++ = *new_ptr++;
   
   /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
   /* Perform a Type IV DCT on the windowed data to get the coefficients */
   /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    dct_type_iv(windowed_data, coefs, dct_size);

}
