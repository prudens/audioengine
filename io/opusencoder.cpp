#include "opusencoder.h"
#include <assert.h>
static void int_to_char( opus_uint32 i, unsigned char ch[4] )
{
    ch[0] = i >> 24;
    ch[1] = ( i >> 16 ) & 0xFF;
    ch[2] = ( i >> 8 ) & 0xFF;
    ch[3] = i & 0xFF;
}
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
     int err;
    outLen = opus_encode( _enc, pcmData, frame_size, (unsigned char*)encodeData + 8, 1500 );
    if ( outLen < 0 )
    {
        return false;
    }
    int nb_encoded = opus_packet_get_samples_per_frame( (unsigned char*)encodeData+8, _samplerate )*opus_packet_get_nb_frames( (unsigned char*)encodeData+8, outLen );
    assert(nb_encoded == frame_size);
    frame_size -= nb_encoded;
    unsigned char *int_field = (unsigned char*)encodeData;
    int_to_char( (opus_int32)outLen, int_field );
    opus_uint32 enc_final_range;
    err = opus_encoder_ctl( _enc, OPUS_GET_FINAL_RANGE( &enc_final_range ) );
    int_field += 4;
    int_to_char( enc_final_range, int_field );
    outLen += 8;

    return true;
}

