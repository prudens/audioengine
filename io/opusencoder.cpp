#include "opusencoder.h"
#include <assert.h>

OPUSEncoder::OPUSEncoder( int samplerate, int16_t channel,int bitrate )
{
    _samplerate = samplerate;
    _channel = channel;
    int err;
    _enc = opus_encoder_create( samplerate, channel, OPUS_APPLICATION_VOIP, &err );
    if ( err != OPUS_OK )
    {
        return;
    }

    opus_encoder_ctl( _enc, OPUS_SET_BITRATE( bitrate ) );
//     opus_encoder_ctl( _enc, OPUS_SET_BANDWIDTH( OPUS_AUTO ) );
     opus_encoder_ctl( _enc, OPUS_SET_VBR( 1 ) );
     opus_encoder_ctl( _enc, OPUS_SET_VBR_CONSTRAINT( 0 ) );
     opus_encoder_ctl( _enc, OPUS_SET_COMPLEXITY( 10 ) );
     opus_encoder_ctl( _enc, OPUS_SET_INBAND_FEC( 1 ) );
//     //opus_encoder_ctl( enc, OPUS_SET_FORCE_CHANNELS( forcechannels ) );
     opus_encoder_ctl( _enc, OPUS_SET_DTX( 1 ) );
     opus_encoder_ctl( _enc, OPUS_SET_PACKET_LOSS_PERC( 50 ) );
     opus_int32 skip;
     opus_encoder_ctl( _enc, OPUS_GET_LOOKAHEAD( &skip ) );
     opus_encoder_ctl( _enc, OPUS_SET_LSB_DEPTH( 16 ) );
//     opus_encoder_ctl( _enc, OPUS_SET_EXPERT_FRAME_DURATION( OPUS_FRAMESIZE_ARG ) );

}

OPUSEncoder::~OPUSEncoder()
{
    opus_encoder_destroy( _enc );
}

void OPUSEncoder::Release()
{
    delete this;
}

bool OPUSEncoder::SetBitRate( int32_t bitRate )
{
    int err = opus_encoder_ctl( _enc, OPUS_SET_BITRATE( bitRate ) );
    return err == OPUS_OK;
}

bool OPUSEncoder::Encode( int16_t* pcmData, int nSamples, char* encodeData, int& outLen )
{
    int frame_size = nSamples/_channel;
    outLen = opus_encode( _enc, pcmData, frame_size, (unsigned char*)encodeData, outLen );
    if ( outLen < 0 )
    {
        return false;
    }
    int nb_encoded = opus_packet_get_samples_per_frame( (unsigned char*)encodeData, _samplerate )*opus_packet_get_nb_frames( (unsigned char*)encodeData, outLen );
    assert(nb_encoded == frame_size);
    return true;
}

