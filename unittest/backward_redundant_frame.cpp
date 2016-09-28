#include <cstring> // import memset
#include <utility> // import swap
#include "backward_redundant_frame.h"


BackwardRedunantFrame::BackwardRedunantFrame()
{
    static_assert( NUM_FRAME == 4, "目前只支持4帧合一" );
    for ( int i = 0; i < NUM_FRAME; i++ )
    {
        auto p = new BRFrame;
        memset( p->data, 0, sizeof( p->data ) );
        brFrames[i] = p;
    }
}

BackwardRedunantFrame::~BackwardRedunantFrame()
{
    for ( int i = 0; i < NUM_FRAME; i++ )
    {
        delete brFrames[i];
    }
}

bool BackwardRedunantFrame::Init( int inSamplerate )
{
    resample24000_.Reset( inSamplerate, BASE_SAMPLE_RATE, 1 );
    return true;
}

void BackwardRedunantFrame::Process( int16_t* data, int nSamples, int16_t* outdata, int &outSample )
{
    size_t outLen = 0;
    int ret = resample24000_.Push( data, 160, brFrames[2]->data, BASE_SAMPLE_RATE / 100, outLen );
    ret = resample24000_.Push( data+160, 160, brFrames[3]->data, BASE_SAMPLE_RATE / 100, outLen );
    for ( int i = 0; i < 240; i++)
    {
        outdata[i*2] = brFrames[0]->data[i];
        outdata[i * 2 + 1] = brFrames[2]->data[i];
    }
    for ( int i = 0; i < 240; i++ )
    {
        outdata[480+i * 2] = brFrames[1]->data[i];
        outdata[480+i * 2 + 1] = brFrames[3]->data[i];
    }

    std::swap( brFrames[0], brFrames[2] );
    std::swap( brFrames[1], brFrames[3] );
    outSample = 480 * 2;
}


