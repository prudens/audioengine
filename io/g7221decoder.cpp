#include <cassert>
#include <string>
#include <cstdint>
#include "g7221decoder.h"
#include "codec/g7221/include/g7221.h"


G7221Decoder::G7221Decoder()
{
    mlt_based_coder_init();
}

void G7221Decoder::Release()
{
    delete this;
}

bool G7221Decoder::Decode( void* encodeData, int outLen, int16_t* pcmData, int &inLen )
{
    if (encodeData && outLen>0)
    {
        DoDecode( encodeData, outLen, pcmData, inLen );
    }
    return true;
}

bool G7221Decoder::DoDecode( void* encodeData, int& outLen, int16_t* pcmData, int& inLen )
{
    assert( inLen/2 >= framesize );
    if ( outLen < 2 * number_of_16bit_words_per_frame )
    {
        outLen = 2 * number_of_16bit_words_per_frame;
        return false;
    }
    decoder( number_of_regions,
             number_of_bits_per_frame,
             (int16_t*)encodeData,
             mlt_coefs,
             frame_error_flag );

    rmlt_coefs_to_samples( mlt_coefs, float_new_samples, framesize );
    {
        float ftemp0;
        for ( int i = 0; i < framesize; i++ )
        {
            ftemp0 = float_new_samples[i];

            if ( ftemp0 >= 0.0 )
            {
                if ( ftemp0 < 32767.0 )
                    pcmData[i] = (int16_t)( ftemp0 + 0.5 );
                else
                    pcmData[i] = 32767;
            }

            else
            {
                if ( ftemp0 > -32768.0 )
                    pcmData[i] = (int16_t)( ftemp0 - 0.5 );
                else
                    pcmData[i] = -32768;
            }
        }
    }
    inLen = framesize*2;
    return true;
}
