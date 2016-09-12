/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : dct4.c
**
**   ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
******************************************************************
*****************************************************************/

/*********************************************************************************
* DCT_TYPE_IV	Discrete Cosine Transform, Type IV
*
* The basis functions are
*
*	 cos(PI*(t+0.5)*(k+0.5)/block_length)
*
* for time t and basis function number k.  Due to the symmetry of the expression
* in t and k, it is clear that the forward and inverse transforms are the same.
*
*********************************************************************************/

/*********************************************************************************
 Include files                                                           
*********************************************************************************/
#include <stdio.h>
#include <math.h>
#include "defs.h"
#include <stdlib.h>
#include <wchar.h>


static float dct_core_a[100];
typedef struct {
    float	cosine;
    float	minus_sine;
    } cos_msin_t;

static cos_msin_t	cos_msin_5[5];      /* Not used since core_size = 10 */
static cos_msin_t	cos_msin_10[10];
static cos_msin_t	cos_msin_20[20];
static cos_msin_t	cos_msin_40[40];
static cos_msin_t	cos_msin_80[80];
static cos_msin_t	cos_msin_160[160];
static cos_msin_t	cos_msin_320[320];
static cos_msin_t	cos_msin_640[640];  /* Used only for 640 point dct */

static cos_msin_t	*cos_msin_table[] = {cos_msin_5,   cos_msin_10,
			                     cos_msin_20,  cos_msin_40,
			                     cos_msin_80,  cos_msin_160,
			                     cos_msin_320, cos_msin_640};


/*********************************************************************************
 Procedure/Function:  set_up_one_table 

 Syntax:       static void set_up_one_table (table, length)
               cos_msin_t	table[];
               long		length; 

 Description:  SET_UP_ONE_TABLE	Set Up One Table of Cosine and Minus Sine Values
               
*********************************************************************************/

static void set_up_one_table (table, length)
     cos_msin_t	table[];
     long		length;
{
  double		angle, scale;
  int		index;

  scale  = PI / (double)(4 * length);
  for (index = 0;    index < length;    index++) {
    angle                   = scale * ((double)index + 0.5);
    table[index].cosine     =  (float) cos(angle);
    table[index].minus_sine = (float) -sin(angle);
  }

}


/*********************************************************************************
 Procedure/Function:  set_up_table s

 Syntax:       static void set_up_tables (long dct_size)
               
 Description:  Set Up Tables of Cosine and Minus Sine Values	
               
*********************************************************************************/
static void set_up_tables(long dct_size)
{
  int length_log;
  int i,k;
  double scale;

  scale = sqrt(2.0/dct_size);

  if ( dct_size <= 0 )
    
	  printf("wrong dct size"), exit(1);

  length_log=0;

  while ( (dct_size&1) == 0)
    {
      length_log++;
      dct_size >>= 1;
    }

  for(k=0;k<10;++k) {
    for(i=0;i<10;++i)
      dct_core_a[10*k+i] = (float)( cos(PI*(k+0.5)*(i+0.5)/10.) * scale); 
  }

  for (i = 0;    i<= length_log ;  i++)
    set_up_one_table (cos_msin_table[i], dct_size<<i);
}


/*********************************************************************************
 Procedure/Function:  dct_type_iv 

 Syntax:       dct_type_iv (input, output, dct_length)
               float	input[], output[];
               long	dct_length;
 
 Description:  Discrete Cosine Transform, Type IV	
               
*********************************************************************************/

void dct_type_iv (input, output, dct_length)
     float	input[], output[];
     long	dct_length;
{
  static int	here_before = 0;
  float		buffer_a[MAX_DCT_SIZE], buffer_b[MAX_DCT_SIZE], buffer_c[MAX_DCT_SIZE];
  float		*in_ptr, *in_ptr_low, *in_ptr_high, *next_in_base;
  float		*out_ptr_low, *out_ptr_high, *next_out_base;
  float		*out_buffer, *in_buffer, *buffer_swap;
  float		in_val_low, in_val_high;
  float		*fptr0, *fptr1, *fptr2;
  float		cos_even, cos_odd, msin_even, msin_odd;
  int		set_span, set_count, set_count_log, pairs_left, sets_left;
  cos_msin_t	**table_ptr_ptr, *cos_msin_ptr;
  int i,k;
  float sum;
  int dct_length_log;
  int core_size;

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Set up the tables if this is the first time here */
/*++++++++++++++++++++++++++++++++++++++++++++++++++*/

  if (here_before == 0) {
    set_up_tables(dct_length);
    here_before = 1;
  }

  dct_length_log=1;
  core_size=dct_length;

  while ( (core_size&1) == 0)
    {
      dct_length_log++;
      core_size >>= 1;
    }

  core_size <<= 1;
  dct_length_log--;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Do the sum/difference butterflies, the first part of */
/* converting one N-point transform into N/2 two-point  */
/* transforms, where N = 1 << dct_length_log.           */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  in_buffer  = input;
  out_buffer = buffer_a;
  for (set_count_log = 0; set_count_log <= dct_length_log - 2; set_count_log++) 
  
  {
    /*===========================================================*/
    /* Initialization for the loop over sets at the current size */
    /*===========================================================*/

    set_span = dct_length>>set_count_log;
   
    set_count     = 1 << set_count_log;
    in_ptr        = in_buffer;
    next_out_base = out_buffer;

    /*=====================================*/
    /* Loop over all the sets of this size */
    /*=====================================*/

    for (sets_left = set_count; sets_left > 0; sets_left--) {

	/*||||||||||||||||||||||||||||||||||||||||||||*/
	/* Set up output pointers for the current set */
	/*||||||||||||||||||||||||||||||||||||||||||||*/

      out_ptr_low    = next_out_base;
      next_out_base += set_span;
      out_ptr_high   = next_out_base;

	/*||||||||||||||||||||||||||||||||||||||||||||||||||*/
	/* Loop over all the butterflies in the current set */
	/*||||||||||||||||||||||||||||||||||||||||||||||||||*/

      do {
	in_val_low      = *in_ptr++;
	in_val_high     = *in_ptr++;
	*out_ptr_low++  = in_val_low + in_val_high;
	*--out_ptr_high = in_val_low - in_val_high;

      } while (out_ptr_low < out_ptr_high);

    } /* End of loop over sets of the current size */

    /*============================================================*/
    /* Decide which buffers to use as input and output next time. */
    /* Except for the first time (when the input buffer is the    */
    /* subroutine input) we just alternate the local buffers.     */
    /*============================================================*/

    in_buffer = out_buffer;
    if (out_buffer == buffer_a)
      out_buffer = buffer_b;
    else
      out_buffer = buffer_a;

  } /* End of loop over set sizes */


/*+++++++++++++++++++++++++++++++++++++*/
/* Do dct_size/10 ten-point transforms */
/*+++++++++++++++++++++++++++++++++++++*/

  fptr0 = in_buffer;
  buffer_swap = buffer_c;

  for (pairs_left = 1 << (dct_length_log - 1);    pairs_left > 0;    pairs_left--) 
    {
      fptr2 = dct_core_a;
      for ( k=0; k<core_size; k++ )
	{
	  fptr1 = fptr0;
	  sum=0;
	  for (i=0; i<core_size; i++)
	    sum += *fptr1++ * *fptr2++;
	  buffer_swap[k] = sum;
	}
      fptr0 += core_size;
      buffer_swap += core_size;
    }

  memcpy(in_buffer, buffer_c, dct_length*sizeof(float));

  table_ptr_ptr = cos_msin_table;

/*++++++++++++++++++++++++++++++*/
/* Perform rotation butterflies */
/*++++++++++++++++++++++++++++++*/

  for (set_count_log = dct_length_log - 2 ;    set_count_log >= 0;    set_count_log--) {

    /*===========================================================*/
    /* Initialization for the loop over sets at the current size */
    /*===========================================================*/

    set_span = dct_length >> set_count_log;
   
    set_count     = 1 << set_count_log;
    next_in_base  = in_buffer;
    if (set_count_log == 0)
      next_out_base = output;
    else
      next_out_base = out_buffer;
    ++table_ptr_ptr;

    /*=====================================*/
    /* Loop over all the sets of this size */
    /*=====================================*/

    for (sets_left = set_count;    sets_left > 0;    sets_left--) {

	/*|||||||||||||||||||||||||||||||||||||||||*/
	/* Set up the pointers for the current set */
	/*|||||||||||||||||||||||||||||||||||||||||*/

	in_ptr_low     = next_in_base;
	in_ptr_high    = in_ptr_low + (set_span >> 1);
	next_in_base  += set_span;
	out_ptr_low    = next_out_base;
	next_out_base += set_span;
	out_ptr_high   = next_out_base;
	cos_msin_ptr   = *table_ptr_ptr;

	/*||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
	/* Loop over all the butterfly pairs in the current set */
	/*||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

	do {
	    cos_even        = (*cos_msin_ptr).cosine;
	    msin_even       = (*cos_msin_ptr++).minus_sine;
	    *out_ptr_low++  = cos_even * *in_ptr_low - msin_even * *in_ptr_high;
	    *--out_ptr_high = msin_even * *in_ptr_low++ +  cos_even * *in_ptr_high++;

	    cos_odd         = (*cos_msin_ptr).cosine;
	    msin_odd        = (*cos_msin_ptr++).minus_sine;
	    *out_ptr_low++  =  cos_odd  * *in_ptr_low  + msin_odd  * *in_ptr_high;
	    *--out_ptr_high = msin_odd  * *in_ptr_low++  -  cos_odd  * *in_ptr_high++;

	    } while (out_ptr_low < out_ptr_high);

	} /* End of loop over sets of the current size */

    /*=============================================*/
    /* Swap input and output buffers for next time */
    /*=============================================*/

    buffer_swap = in_buffer;
    in_buffer   = out_buffer;
    out_buffer  = buffer_swap;
    }

}
