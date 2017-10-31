#include "opusdecoder.h"
#include <stdio.h>
#include <string>
namespace audio_engine
{
	OPUSDecoder::OPUSDecoder( int sampling_rate, int channels )
	{
		samplerate = sampling_rate;
		channel = channels;
		int err;
		dec = opus_decoder_create( sampling_rate, channels, &err );
		if(err != 0)
		{
			fprintf( stderr, "Cannot create decoder: %s\n", opus_strerror( err ) );
			return;
		}
		opus_decoder_ctl( dec, OPUS_SET_INBAND_FEC( 1 ) );
	}

	OPUSDecoder::~OPUSDecoder()
	{
		opus_decoder_destroy( dec );
	}

	void OPUSDecoder::Release()
	{
		delete this;
	}

	bool OPUSDecoder::Decode( void* encodeData, int encLen, int16_t* pcmData, int& nSamples )
	{
		int frame_size = 0;
		int output_samples;
		bool lost = ( encLen == 0 );
		if(lost)
			opus_decoder_ctl( dec, OPUS_GET_LAST_PACKET_DURATION( &output_samples ) );
		else
			output_samples = 96000;
		if(count >= use_inbandfec)
		{
			/* delay by one packet when using in-band FEC */
			if(use_inbandfec)
			{
				if(lost_prev)
				{
					/* attempt to decode with in-band FEC from next packet */
					opus_decoder_ctl( dec, OPUS_GET_LAST_PACKET_DURATION( &output_samples ) );
					output_samples = opus_decode( dec, lost ? NULL : (unsigned char*)encodeData, encLen, pcmData, output_samples, 1 );
					frame_size = output_samples;
				}
				else
				{
					/* regular decode */
					output_samples = 96000;
					output_samples = opus_decode( dec, prevData, prevLen, pcmData, output_samples, 0 );
					frame_size = output_samples;
				}
			}
			else
			{
				output_samples = opus_decode( dec, lost ? NULL : (unsigned char*)encodeData, encLen, pcmData, output_samples, 0 );
				frame_size = output_samples;
			}
		}
		count++;
		if(encLen > 0)
		{
			memcpy( prevData, encodeData, encLen );
		}
		lost_prev = lost;
		prevLen = encLen;
		nSamples = frame_size * channel;
		return true;
	}

}