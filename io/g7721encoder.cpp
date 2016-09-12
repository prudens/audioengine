#include <cassert>
#include <string>
#include "g7721encoder.h"
// C++调用C函数需要加extern "C"
extern "C"
{
#include "codec/g7221/include/g7221.h"
}

G7221Encoder::G7221Encoder()
{
    mlt_based_coder_init();
}

void G7221Encoder::Release()
{
    delete this;
}

bool G7221Encoder::Encode( int16_t* pcmData, int inLen, void* encodeData, int& outLen )
{
    if ( need_size > inLen/2)
    {
        memcpy( input + framesize - need_size, pcmData, inLen );
        need_size -= inLen/2;
        outLen = 0;
        return true;
    }
    else
    {
        memcpy( input + framesize - need_size, pcmData, need_size*2 );
        need_size = framesize - (inLen / 2 - need_size);
        DoEncode( input, framesize, encodeData, outLen );
        
    }
    return true;
}

bool G7221Encoder::DoEncode( int16_t* pcmData, int inLen, void* encodeData, int& outLen )
{
    assert( inLen == framesize );
    if ( outLen < 2 * number_of_16bit_words_per_frame )
    {
        outLen = 2 * number_of_16bit_words_per_frame;
        return false;
    }
    for ( int i = 0; i < framesize; i++ )
    {
        float_new_samples[i] = (float)pcmData[i];
    }

    samples_to_rmlt_coefs( float_new_samples, mlt_coefs, framesize );
    /*
    **	This was added to for fixed point interop
    */
    for ( int i = 0; i < framesize; i++ )
    {
        mlt_coefs[i] /= 22.0f;
    }

    /* Encode the mlt coefs  */
    encoder( number_of_regions,
             number_of_bits_per_frame,
             mlt_coefs,
             (short*)encodeData );
    outLen = 2 * number_of_16bit_words_per_frame;
    return true;
}
