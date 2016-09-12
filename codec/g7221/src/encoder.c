/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : encoder.c
**
**   ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
**************************************************************************/
/***************************************************************************
  Filename:   encoder.c    
  Original Author:
  Creation Date:
  Purpose:         Contains files used to implement
                   the G.722.1 encoder  
***************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/

#include <stdio.h>
#include <math.h>
#include "../include/defs.h"
#include "huff_defs.h"
#include "../include/g7221.h"
/* local function declarations */
int compute_region_powers(int, float [], int [], int [], int[]);
void vector_quantize_mlts(int, int, float [], int[], int [], int [], int *,
			  int [], int []);
int vector_huffman(int , int, float *, int *);

extern float region_standard_deviation_table[REGION_POWER_TABLE_SIZE];
extern float standard_deviation_inverse_table[REGION_POWER_TABLE_SIZE];
extern float step_size_inverse_table[NUM_CATEGORIES];
extern float dead_zone[NUM_CATEGORIES];
extern int region_size;
extern float region_size_inverse;
extern float region_power_table[REGION_POWER_TABLE_SIZE];
extern float region_power_table_boundary[REGION_POWER_TABLE_SIZE-1];
extern int differential_region_power_bits[MAX_NUM_REGIONS][DIFF_REGION_POWER_LEVELS];
extern int differential_region_power_codes[MAX_NUM_REGIONS][DIFF_REGION_POWER_LEVELS];
extern int table_of_bitcount_tables[NUM_CATEGORIES-1];
extern int table_of_code_tables[NUM_CATEGORIES-1];
extern int vector_dimension[NUM_CATEGORIES];
extern int number_of_vectors[NUM_CATEGORIES];
extern float step_size[NUM_CATEGORIES];
extern int max_bin[NUM_CATEGORIES];

static int num_rate_control_bits;
static int num_rate_control_possibilities;

/***************************************************************************
 Procedure/Function:  encoder                                                     
                                                                         
 Syntax:      void encoder( number_of_regions,
							number_of_available_bits,
							mlt_coefs,
							out_words)
							int number_of_regions;
							int number_of_available_bits;
							float mlt_coefs[MAX_DCT_SIZE];
							short int out_words[MAX_BITS_PER_FRAME/16];
             
              inputs:   number_of_regions
						number_of_available_bits
                        mlt_coefs[MAX_DCT_SIZE]
                        
              outputs:  out_words[MAX_BITS_PER_FRAME/16]
                                                                         
                                                                         
 Description:  Encodes the mlt coefs into out_words using the G.722.1 algorithm
                                                                         
***************************************************************************/

void encoder(number_of_regions,
	     number_of_available_bits,
	     mlt_coefs,
	     out_words)
     int number_of_regions;
     int number_of_available_bits;
     float mlt_coefs[MAX_DCT_SIZE];
     short int out_words[MAX_BITS_PER_FRAME/16];

{
  extern void categorize(int, int, int[], int[], int[]);

  int number_of_bits_per_frame;
  int number_of_envelope_bits;
  int rate_control;
  int region;
  int absolute_region_power_index[MAX_NUM_REGIONS];
  int power_categories[MAX_NUM_REGIONS];
  int category_balances[MAX_NUM_RATE_CONTROL_POSSIBILITIES-1];
  int drp_num_bits[MAX_NUM_REGIONS+1];
  int drp_code_bits[MAX_NUM_REGIONS+1];

  int region_mlt_bit_counts[MAX_NUM_REGIONS];
  int region_mlt_bits[4*MAX_NUM_REGIONS];

  if (number_of_regions <= 14) {
    num_rate_control_bits = 4;
    num_rate_control_possibilities = 16;
  }




  number_of_bits_per_frame = number_of_available_bits;

/* Estimate power envelope. */

  number_of_envelope_bits = compute_region_powers(number_of_regions,
						  mlt_coefs,
						  drp_num_bits,
						  drp_code_bits,
						  absolute_region_power_index);

  number_of_available_bits -= number_of_envelope_bits;
  number_of_available_bits -= num_rate_control_bits;

  categorize(number_of_regions,
	     number_of_available_bits,
	     absolute_region_power_index,
	     power_categories,
	     category_balances);

/* Adjust absolute_region_category_index[] for mag_shift.
   This assumes that REGION_POWER_STEPSIZE_DB is defined
   to be exactly 3.010299957 or 20.0 times log base 10
   of square root of 2. */

  for (region=0; region<number_of_regions; region++)
    absolute_region_power_index[region] += REGION_POWER_TABLE_NUM_NEGATIVES;


  for (region=0; region<number_of_regions; region++)
    region_mlt_bit_counts[region] = 0;


  vector_quantize_mlts(number_of_regions,
		       number_of_available_bits,
		       mlt_coefs,
		       absolute_region_power_index,
		       power_categories,
		       category_balances,
		       &rate_control,
		       region_mlt_bit_counts,
		       region_mlt_bits);

  {
    int out_word_index;
    int j;
    short int out_word;
    int region_bit_count;
    int current_word_bits_left;
    short int slice;

    int out_word_bits_free;

    int *in_word_ptr;
    unsigned int current_word;

    out_word_index = 0;



    out_word = 0;
    out_word_bits_free = 16;



/* First set up the rate control bits to look like one more set of region power bits. */
    
	drp_num_bits[number_of_regions] = num_rate_control_bits;
    drp_code_bits[number_of_regions] = rate_control;

/* These code bits are right justified. */
    
	for (region=0; region <= number_of_regions; region++) {
      current_word_bits_left = drp_num_bits[region];
      current_word = drp_code_bits[region];
      j = current_word_bits_left-out_word_bits_free;
      if (j >= 0) {
	out_word += current_word >> j;
	out_words[out_word_index++] = out_word;
	out_word_bits_free = 16;
	out_word = current_word << (16 - j);
	out_word_bits_free -= j;
      }
      else {
	out_word += current_word << (-j);
	out_word_bits_free -= current_word_bits_left;
      }
    }

/* These code bits are left justified. */
    
	for (region=0; (region<number_of_regions) && ((16*out_word_index)<number_of_bits_per_frame); region++) {
      in_word_ptr = &region_mlt_bits[4*region];
      region_bit_count = region_mlt_bit_counts[region];
      current_word_bits_left = MIN(32, region_bit_count);
      current_word = *in_word_ptr++;
      while ((region_bit_count > 0) && ((16*out_word_index)<number_of_bits_per_frame)) {
	if (current_word_bits_left >= out_word_bits_free) {
	  slice = current_word >> (32 - out_word_bits_free);
	  out_word += slice;
	  current_word <<= out_word_bits_free;
	  current_word_bits_left -= out_word_bits_free;
	  out_words[out_word_index++] = out_word;
	  out_word = 0;
	  out_word_bits_free = 16;
	}
	else {
	  slice = current_word >> (32 - current_word_bits_left);
	  out_word += slice << (out_word_bits_free - current_word_bits_left);
	  out_word_bits_free -= current_word_bits_left;
	  current_word_bits_left = 0;
	}
	if (current_word_bits_left == 0){
	  current_word = *in_word_ptr++;
	  region_bit_count -= 32;
	  current_word_bits_left = MIN(32, region_bit_count);
	}
      }
    }

/* Fill out with 1's. */
    
	while ((16*out_word_index)<number_of_bits_per_frame) {
      current_word = 0x0000ffff;
      slice = current_word >> (16 - out_word_bits_free);
      out_word += slice;
      out_words[out_word_index++] = out_word;
      out_word = 0;
      out_word_bits_free = 16;
    }
  }
}

/***************************************************************************
 Procedure/Function:  compute_region_powers

 Syntax:   int compute_region_powers( number_of_regions,
									  mlt_coefs,
									  drp_num_bits,
									  drp_code_bits,
									  absolute_region_power_index)
									 int number_of_regions;
									 float mlt_coefs[MAX_DCT_SIZE];
									 int drp_num_bits[MAX_NUM_REGIONS];
									 int drp_code_bits[MAX_NUM_REGIONS];
									 int absolute_region_power_index[MAX_NUM_REGIONS];
                                                                   
 Description:   Computes the power for each of the regions

***************************************************************************/


int compute_region_powers(number_of_regions,
			  mlt_coefs,
			  drp_num_bits,
			  drp_code_bits,
			  absolute_region_power_index)
     int number_of_regions;
     float mlt_coefs[MAX_DCT_SIZE];
     int drp_num_bits[MAX_NUM_REGIONS];
     int drp_code_bits[MAX_NUM_REGIONS];
     int absolute_region_power_index[MAX_NUM_REGIONS];

{

  float *input_ptr;
  int iterations;
  float ftemp0, ftemp1;
  int index, index_min, index_max;

  int region;
  int j;
  int differential_region_power_index[MAX_NUM_REGIONS];
  int number_of_bits;

  input_ptr = mlt_coefs;
  for (region=0; region<number_of_regions; region++) {
    ftemp0 = 0.0;
    for (j=0; j<region_size; j++) {
      ftemp1 = *input_ptr++;
      ftemp0 += ftemp1*ftemp1;
    }
    ftemp0 *= region_size_inverse;


    index_min = 0;
    index_max = REGION_POWER_TABLE_SIZE;
    for (iterations = 0; iterations < 6; iterations++) {
      index = (index_min + index_max) >> 1;
      if (ftemp0 < region_power_table_boundary[index-1])
	index_max = index;
      else
	index_min = index;
    }
    absolute_region_power_index[region] = index_min-REGION_POWER_TABLE_NUM_NEGATIVES;
  }


/* Before we differentially encode the quantized region powers, adjust upward the
   valleys to make sure all the peaks can be accurately represented. */

  for (region = number_of_regions-2; region >= 0; region--) {
    if (absolute_region_power_index[region] < absolute_region_power_index[region+1] - DRP_DIFF_MAX)
      absolute_region_power_index[region] = absolute_region_power_index[region+1] - DRP_DIFF_MAX;
  }

/* The MLT is currently scaled too low by the factor
   ENCODER_SCALE_FACTOR(=18318)/32768 * (1./sqrt(160).
   This is the ninth power of 1 over the square root of 2.
   So later we will add ESF_ADJUSTMENT_TO_RMS_INDEX (now 9)
   to drp_code_bits[0]. */

/* drp_code_bits[0] can range from 1 to 31. 0 will be used only as an escape sequence. */
  
  if (absolute_region_power_index[0] < 1-ESF_ADJUSTMENT_TO_RMS_INDEX)
    absolute_region_power_index[0] = 1-ESF_ADJUSTMENT_TO_RMS_INDEX;
  if (absolute_region_power_index[0] > 31-ESF_ADJUSTMENT_TO_RMS_INDEX)
    absolute_region_power_index[0] = 31-ESF_ADJUSTMENT_TO_RMS_INDEX;

  differential_region_power_index[0] = absolute_region_power_index[0];
  number_of_bits = 5;
  drp_num_bits[0] = 5;
  drp_code_bits[0] = absolute_region_power_index[0] + ESF_ADJUSTMENT_TO_RMS_INDEX;

/* Lower limit the absolute region power indices to -8 and upper limit them to 31. Such extremes
 may be mathematically impossible anyway.*/
  
  for (region=1; region<number_of_regions; region++) {
    if (absolute_region_power_index[region] < -8 -ESF_ADJUSTMENT_TO_RMS_INDEX)
      absolute_region_power_index[region] = -8-ESF_ADJUSTMENT_TO_RMS_INDEX;
    if (absolute_region_power_index[region] > 31-ESF_ADJUSTMENT_TO_RMS_INDEX)
      absolute_region_power_index[region] = 31-ESF_ADJUSTMENT_TO_RMS_INDEX;
  }

  for (region=1; region<number_of_regions; region++) {
    j = absolute_region_power_index[region] - absolute_region_power_index[region-1];
    if (j < DRP_DIFF_MIN) {
      j = DRP_DIFF_MIN;
    }
    j -= DRP_DIFF_MIN;
    differential_region_power_index[region] = j;
    absolute_region_power_index[region] = absolute_region_power_index[region-1] +
      differential_region_power_index[region] + DRP_DIFF_MIN;

    number_of_bits += differential_region_power_bits[region][j];
    drp_num_bits[region] = differential_region_power_bits[region][j];
    drp_code_bits[region] = differential_region_power_codes[region][j];
  }

  return(number_of_bits);
}

/***************************************************************************
 Procedure/Function:  vector_quantize_mlts

 Syntax:    void vector_quantize_mlts(number_of_regions,
			  number_of_available_bits,
			  mlt_coefs,
			  absolute_region_power_index,
			  power_categories,
			  category_balances,
			  p_rate_control,
			  region_mlt_bit_counts,
			  region_mlt_bits)

     int number_of_regions;
     int number_of_available_bits;
     float mlt_coefs[MAX_DCT_SIZE];
     int absolute_region_power_index[MAX_NUM_REGIONS];
     int power_categories[MAX_NUM_REGIONS];
     int category_balances[MAX_NUM_RATE_CONTROL_POSSIBILITIES-1];
     int *p_rate_control;
     int region_mlt_bit_counts[MAX_NUM_REGIONS];
     int region_mlt_bits[4*MAX_NUM_REGIONS];

 Description:  

***************************************************************************/

void vector_quantize_mlts(number_of_regions,
			  number_of_available_bits,
			  mlt_coefs,
			  absolute_region_power_index,
			  power_categories,
			  category_balances,
			  p_rate_control,
			  region_mlt_bit_counts,
			  region_mlt_bits)

     int number_of_regions;
     int number_of_available_bits;
     float mlt_coefs[MAX_DCT_SIZE];
     int absolute_region_power_index[MAX_NUM_REGIONS];
     int power_categories[MAX_NUM_REGIONS];
     int category_balances[MAX_NUM_RATE_CONTROL_POSSIBILITIES-1];
     int *p_rate_control;
     int region_mlt_bit_counts[MAX_NUM_REGIONS];
     int region_mlt_bits[4*MAX_NUM_REGIONS];

{

  float *raw_mlt_ptr;
  int region;
  int category;
  int total_mlt_bits;

  total_mlt_bits = 0;

/* Start in the middle of the rate control range. */

  for (*p_rate_control = 0; *p_rate_control < ((num_rate_control_possibilities >> 1) - 1);
       (*p_rate_control)++) {
    region = category_balances[*p_rate_control];
    power_categories[region]++;
  }

  for (region=0; region<number_of_regions; region++) {
    category = power_categories[region];
    raw_mlt_ptr = &mlt_coefs[region*region_size];
    if (category < NUM_CATEGORIES-1)
      {
	region_mlt_bit_counts[region] =
	  vector_huffman(category, absolute_region_power_index[region],raw_mlt_ptr,
			 &region_mlt_bits[4*region]);
      }
    else
      {
	region_mlt_bit_counts[region] = 0;
      }
    total_mlt_bits += region_mlt_bit_counts[region];
  }


/* If too few bits... */

  while ((total_mlt_bits < number_of_available_bits) && (*p_rate_control > 0)) {
    (*p_rate_control)--;
    region = category_balances[*p_rate_control];
    power_categories[region]--;
    total_mlt_bits -= region_mlt_bit_counts[region];

    category = power_categories[region];
    raw_mlt_ptr = &mlt_coefs[region*region_size];
    if (category < NUM_CATEGORIES-1)
      {
	region_mlt_bit_counts[region] =
	  vector_huffman(category, absolute_region_power_index[region],raw_mlt_ptr,
			 &region_mlt_bits[4*region]);
      }
    else
      {
	region_mlt_bit_counts[region] = 0;
      }
    total_mlt_bits += region_mlt_bit_counts[region];
  }

/* If too many bits... */
  
  while ((total_mlt_bits > number_of_available_bits) && (*p_rate_control < num_rate_control_possibilities-1)) {
    region = category_balances[*p_rate_control];
    power_categories[region]++;
    total_mlt_bits -= region_mlt_bit_counts[region];

    category = power_categories[region];
    raw_mlt_ptr = &mlt_coefs[region*region_size];
    if (category < NUM_CATEGORIES-1)
      {
	region_mlt_bit_counts[region] =
	  vector_huffman(category, absolute_region_power_index[region],raw_mlt_ptr,
			 &region_mlt_bits[4*region]);
      }
    else
      {
	region_mlt_bit_counts[region] = 0;
      }
    total_mlt_bits += region_mlt_bit_counts[region];
    (*p_rate_control)++;
  }

}

/***************************************************************************
 Procedure/Function:  vector_huffman

 Syntax:    int vector_huffman(category,
							   power_index,
							   raw_mlt_ptr,
							   word_ptr)

							 int category;
							 int power_index;
							 float *raw_mlt_ptr;
							 int *word_ptr;     

             inputs:     int  category
                         int  power_index
                         float  *raw_mlt_ptr
             
             outputs:    int    *word_ptr
                                      

 Description:  Huffman encoding for each region based on category and power_index  

***************************************************************************/

int vector_huffman(category,
		   power_index,
		   raw_mlt_ptr,
		   word_ptr)

     int category;
     int power_index;
     float *raw_mlt_ptr;
     int *word_ptr;
{
  float inv_of_step_size_times_std_dev;
  int j,n;
  int k;
  int number_of_region_bits;
  int number_of_non_zero;
  int vec_dim;
  int num_vecs;
  int kmax, kmax_plus_one;
  int index,signs_index;
  int *bitcount_table_ptr;
  int *code_table_ptr;
  int code_bits;
  int number_of_code_bits;
  int current_word;
  int current_word_bits_free;

  vec_dim = vector_dimension[category];
  num_vecs = number_of_vectors[category];
  kmax = max_bin[category];
  kmax_plus_one = kmax + 1;

  current_word = 0;
  current_word_bits_free = 32;

  number_of_region_bits = 0;

  bitcount_table_ptr = (int *) table_of_bitcount_tables[category];
  code_table_ptr = (int *) table_of_code_tables[category];


  inv_of_step_size_times_std_dev = 
    step_size_inverse_table[category]*standard_deviation_inverse_table[power_index];

  for (n=0; n<num_vecs; n++) {

    index = 0;
    signs_index = 0;
    number_of_non_zero = 0;
    for (j=0; j<vec_dim; j++) {


      k = (int) (fabs(*raw_mlt_ptr) * inv_of_step_size_times_std_dev + dead_zone[category]);

      if (k != 0) {
	number_of_non_zero++;
	signs_index <<= 1;
	if (*raw_mlt_ptr > 0)
	  signs_index++;
	if (k > kmax)
	  k = kmax;
      }
      index = index*(kmax_plus_one) + k;
      raw_mlt_ptr++;
    }

    code_bits = *(code_table_ptr+index);
    number_of_code_bits = *(bitcount_table_ptr+index) + number_of_non_zero;
    number_of_region_bits += number_of_code_bits;

    code_bits = (code_bits << number_of_non_zero) + signs_index;

  /* msb of codebits is transmitted first. */

    j = current_word_bits_free - number_of_code_bits;
    if (j >= 0) {
      current_word += code_bits << j;
      current_word_bits_free = j;
    }
    else {
      j = -j;
      current_word += code_bits >> j;
      *word_ptr++ = current_word;
      current_word_bits_free = 32-j;
      current_word = code_bits << current_word_bits_free;
    }

  }

  *word_ptr++ = current_word;

  return(number_of_region_bits);
}


