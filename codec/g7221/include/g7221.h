#pragma once
/****************************************************************************************
Procedure/Function:  mlt_based_coder_init

Syntax:        void mlt_based_coder_init()
inputs:  none

outputs: none

Description:   Initializes region and category related stuff

****************************************************************************************/
void mlt_based_coder_init();


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

void samples_to_rmlt_coefs( float * new_samples,
                            float * coefs,
                            int dct_size );


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

void encoder( int number_of_regions,
              int number_of_available_bits,
              float* mlt_coefs,
              short int *out_words );


