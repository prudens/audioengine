/*****************************************************************
******************************************************************
**
**   G.722.1 Annex B - G.722.1 Floating point implementation
**   > Software Release 2.1 (2008-06)
**
**	Filename : common.c
**
**    ?2000 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
******************************************************************
*****************************************************************/

#include <stdio.h>
#include <math.h>
#include "defs.h"

#include "huff_defs.h"
#include "huff_tables.h"

int region_size;
float region_size_inverse;

float region_standard_deviation_table[REGION_POWER_TABLE_SIZE];
float standard_deviation_inverse_table[REGION_POWER_TABLE_SIZE];
float step_size_inverse_table[NUM_CATEGORIES];


float region_power_table[REGION_POWER_TABLE_SIZE];
float region_power_table_boundary[REGION_POWER_TABLE_SIZE-1];

int vector_dimension[NUM_CATEGORIES] =  { 2, 2, 2, 4, 4, 5, 5, 1};
int number_of_vectors[NUM_CATEGORIES] = {10,10,10, 5, 5, 4, 4,20};

/* The last category isn't really coded with scalar quantization. */

float step_size[NUM_CATEGORIES] = {0.3536, 0.5, 0.7071, 1.0, 1.4142, 2.0, 2.8284, 2.8284};
int max_bin[NUM_CATEGORIES] = {13, 9, 6, 4, 3, 2, 1, 1};
int int_dead_zone[NUM_CATEGORIES];
float dead_zone[NUM_CATEGORIES] = {0.3, 0.33, 0.36, 0.39, 0.42, 0.45, 0.5, 0.5};
static int max_bin_plus_one_inverse[NUM_CATEGORIES];

/****************************************************************************************
 Procedure/Function:  index_to_array 

 Syntax:   number_of_non_zero = index_to_array(int index, 
                                               int array[MAX_VECTOR_DIMENSION],
                                               int category)

                inputs:  int index
                         int category                     
                       
                outputs: int array[MAX_VECTOR_DIMENSION] - used in decoder to access
                                                             mlt_quant_centroid table
                        
                         int number_of_non_zero          - number of non zero elements
                                                             in the array
 
 Description:   Computes an array of sign bits with the length of the category vector
                Returns the number of sign bits and the array

****************************************************************************************/

int index_to_array(index,array,category)
int index;
int array[MAX_VECTOR_DIMENSION];
int category;
{
  int j,q,p;
  int number_of_non_zero;
  int max_bin_plus_one;
  int inverse_of_max_bin_plus_one;

  number_of_non_zero = 0;
  p = index;
  max_bin_plus_one = max_bin[category] + 1;
  inverse_of_max_bin_plus_one = max_bin_plus_one_inverse[category];

  for (j=vector_dimension[category]-1; j>=0; j--) {

	  /*    q = p/max_bin_plus_one; */
    
	  q = (p*inverse_of_max_bin_plus_one) >> 15;
    array[j] = p - q*max_bin_plus_one;
    p = q;
    if (array[j] != 0) number_of_non_zero++;
  }
  return(number_of_non_zero);
}

/****************************************************************************************
 Procedure/Function:  mlt_based_coder_init

 Syntax:        void mlt_based_coder_init()
                inputs:  none
                         
                outputs: none
 
 Description:   Initializes region and category related stuff

****************************************************************************************/
 void mlt_based_coder_init()
	 {
  int i,j;
  int category;
  int number_of_indices;
 
/*  region_size = (BLOCK_SIZE * 0.875)/NUM_REGIONS; */
  
  region_size = 20;
  region_size_inverse = (float)(1.0/((float) region_size));

  for (i=0; i<REGION_POWER_TABLE_SIZE; i++) {
    region_power_table[i] =
      (float) pow(10.0, 0.10*REGION_POWER_STEPSIZE_DB*
	  ((float) (i-REGION_POWER_TABLE_NUM_NEGATIVES)));

    region_standard_deviation_table[i] = (float)sqrt(region_power_table[i]);
    standard_deviation_inverse_table[i] = (float) 1.0/region_standard_deviation_table[i];

  }

  for (i=0; i<REGION_POWER_TABLE_SIZE-1; i++)
    region_power_table_boundary[i] = (float)
      pow(10.0, 0.10*REGION_POWER_STEPSIZE_DB*
	  (0.5+((float) (i-REGION_POWER_TABLE_NUM_NEGATIVES))));

  /* Initialize category related stuff. */

  for (category=0; category<NUM_CATEGORIES; category++) {

  /* Rounding up by 1.0 instead of 0.5 allows us to avoid rounding every
   time this is used. */

	  max_bin_plus_one_inverse[category] = (int) ((32768./(max_bin[category]+1.0)) + 1.0);

/* Test division for all indices. */

    number_of_indices = 1;
    for (j=0; j<vector_dimension[category]; j++)
      number_of_indices *= (max_bin[category]+1);
    for (j=0; j<number_of_indices; j++) {
      if (j/(max_bin[category]+1) != ((j*max_bin_plus_one_inverse[category]) >> 15))
	printf("max_bin_plus_one_inverse ERROR!! %1d: %5d %3d\n",category,max_bin_plus_one_inverse[category],j);
    }
  }

  for (category=0; category<NUM_CATEGORIES; category++)
    step_size_inverse_table[category] = (float) 1.0/step_size[category];

}

/****************************************************************************************
 Procedure/Function:  categorize

 Syntax:    void categorize(number_of_regions,
                            number_of_available_bits,   
                            rms_index,                  
                            power_categories,           
                            category_balances)          

                  inputs:   number_of_regions
				            number_of_available_bits
                            rms_index[MAX_NUM_REGIONS]                              
                  
                  outputs:  power_categories[MAX_NUM_REGIONS]                       
                            category_balances[MAX_NUM_RATE_CONTROL_POSSIBILITIES-1]

 Description: Computes a series of categorizations    			
****************************************************************************************/

	void categorize(number_of_regions,
		number_of_available_bits,
		rms_index,
		power_categories,
		category_balances)
     int number_of_regions;
     int number_of_available_bits;
     int rms_index[MAX_NUM_REGIONS];
     int power_categories[MAX_NUM_REGIONS];
     int category_balances[MAX_NUM_RATE_CONTROL_POSSIBILITIES-1];

	 {
  int region;
  int j;
  int expected_number_of_code_bits;

  int delta;
  int offset;
  int test_offset;

  int num_rate_control_possibilities;

  if (number_of_regions <= 14)
    num_rate_control_possibilities = 16;


/* At higher bit rates, there is an increase for most categories in average bit
   consumption per region. We compensate for this by pretending we have fewer
   available bits. 
*/

  if (number_of_regions <= 14) 
  {
    if (number_of_available_bits > 320)
      number_of_available_bits =
	320 + (((number_of_available_bits - 320)*5) >> 3);
  }

  offset = -32;
  delta = 32;
  do {
    test_offset = offset + delta;
    for (region=0; region<number_of_regions; region++) {
      j = (test_offset-rms_index[region]) >> 1;
      if (j < 0) j = 0;
      if (j > NUM_CATEGORIES-1) j = NUM_CATEGORIES-1;
      power_categories[region] = j;
    }
    expected_number_of_code_bits = 0;
    for (region=0; region<number_of_regions; region++)
      expected_number_of_code_bits += expected_bits_table[power_categories[region]];

    if (expected_number_of_code_bits >= number_of_available_bits - 32)
      offset = test_offset;

    delta >>= 1;
  } while (delta > 0);


  for (region=0; region<number_of_regions; region++) {
    j = (offset-rms_index[region]) >> 1;
    if (j < 0) j = 0;
    if (j > NUM_CATEGORIES-1) j = NUM_CATEGORIES-1;
    power_categories[region] = j;
  }
  expected_number_of_code_bits = 0;
  for (region=0; region<number_of_regions; region++)
    expected_number_of_code_bits += expected_bits_table[power_categories[region]];


  {
    int max_rate_categories[MAX_NUM_REGIONS];
    int min_rate_categories[MAX_NUM_REGIONS];
    int temp_category_balances[2*MAX_NUM_RATE_CONTROL_POSSIBILITIES];

    int raw_max, raw_min;
    int raw_max_index, raw_min_index;
    int max_rate_pointer, min_rate_pointer;
    int max, min;
    int itemp0;

    for (region=0; region<number_of_regions; region++) {
      max_rate_categories[region] = power_categories[region];
      min_rate_categories[region] = power_categories[region];
    }

    max = expected_number_of_code_bits;
    min = expected_number_of_code_bits;
    max_rate_pointer = num_rate_control_possibilities;
    min_rate_pointer = num_rate_control_possibilities;

    for (j=0; j<num_rate_control_possibilities-1; j++) {
      if (max+min <= 2*number_of_available_bits) {
	raw_min = 99;

	/* Search from lowest freq regions to highest for best region to reassign to
   a higher bit rate category. */

	for (region=0; region<number_of_regions; region++) {
	  if (max_rate_categories[region] > 0) {
	    itemp0 = offset - rms_index[region] - 2*max_rate_categories[region];
	    if (itemp0 < raw_min) {
	      raw_min = itemp0;
	      raw_min_index = region;
	    }
	  }
	}
	max_rate_pointer--;
	temp_category_balances[max_rate_pointer] = raw_min_index;

	max -= expected_bits_table[max_rate_categories[raw_min_index]];
	max_rate_categories[raw_min_index] -= 1;
	max += expected_bits_table[max_rate_categories[raw_min_index]];
      }

      else {
	raw_max = -99;

	/* Search from highest freq regions to lowest for best region to reassign to
   a lower bit rate category. */

	for (region=number_of_regions-1; region >= 0; region--) {
	  if (min_rate_categories[region] < NUM_CATEGORIES-1){
	    itemp0 = offset - rms_index[region] - 2*min_rate_categories[region];
	    if (itemp0 > raw_max) {
	      raw_max = itemp0;
	      raw_max_index = region;
	    }
	  }
	}
	temp_category_balances[min_rate_pointer] = raw_max_index;
	min_rate_pointer++;

	min -= expected_bits_table[min_rate_categories[raw_max_index]];
	min_rate_categories[raw_max_index] += 1;
	min += expected_bits_table[min_rate_categories[raw_max_index]];
      }
    }

    for (region=0; region<number_of_regions; region++)
      power_categories[region] = max_rate_categories[region];

    for (j=0; j<num_rate_control_possibilities-1; j++)
      category_balances[j] = temp_category_balances[max_rate_pointer++];

  }

}
