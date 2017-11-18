#include "codec_converter.h"
#include "io/include/io_common_define.h"

CodecFactory::CodecFactory()
{
}


CodecConverter* CodecFactory::Create( int src_type, int dst_type, int samplerate, int channel )
{
	if ( src_type == AFT_PCM )
	{
		switch( dst_type )
		{
		case AFT_AAC:
		{
			CodecConverter* converter = new CodecConverter();
		}
		break;
		case AFT_OPUS:
			break;
		default:
			return nullptr;
		}
	}
}

void CodecFactory::SetHardwareCodec( bool open )
{
    _hardware_codec = open;
}
bool CodecFactory::IsHardwareCodec()const
{
    return _hardware_codec;
}
