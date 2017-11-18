#pragma once
#include "codec_converter.h"
#include "../aac/3rd/inc/encoder.h"
#include "../aac/3rd/inc/decoder.h"
//#include "neaacdec.h"
#include <algorithm>

class AACCodecFAAC : public CodecConverter
{
    bool is_encoder;
public:
    AACCodecFAAC( bool encoder )
    {
        is_encoder = encoder;

    }
    ~AACCodecFAAC()
    {
        if ( m_inst )
        {
            if ( is_encoder )
            {
                bodtech::acodec::enc::close( m_inst );
            }
            else
            {
                bodtech::acodec::dec::close( m_inst );
            }
        }
    }

    bool Init( int profile, int samplerate, int channel, int bitrate )
    {

        if ( is_encoder )
        {
            m_inst = bodtech::acodec::enc::open( samplerate, channel, bitrate );
        }
        else
        {
            m_inst = bodtech::acodec::dec::open( samplerate, channel );
        }
        return m_inst != nullptr;
    }
    virtual int Process( const void* inputData, size_t inputSize, void* outputData, size_t& outputSize )
    {
        if (m_inst == nullptr)
        {
            return -1;
        }
        if ( is_encoder )
        {
            outputSize = bodtech::acodec::enc::encode( m_inst, (void*)inputData, inputSize, outputData, outputSize );
        }
        else
        {
            outputSize = bodtech::acodec::dec::decode( m_inst, (void*)inputData, inputSize, outputData, outputSize );
        }
        return 0;
    }
    virtual void Destroy()
    {
        delete this;
    }
private:
    void* m_inst = nullptr;
};
