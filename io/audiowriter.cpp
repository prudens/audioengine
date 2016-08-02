#include "io/include/audiowriter.h"
#include "aacfilewriter.h"
#include "mp3filewriter.h"
AudioWriter*AudioWriter::Create( const char* filename, int sample_rate, size_t channels, AudioFileType type )
{
    if ( type == AFT_MP3 )
    {
        return new Mp3FileWriter( filename, sample_rate, channels );
    }
    else if ( type == AFT_AAC )
    {
        return new AACFileWriter( filename, sample_rate, channels );
    }
    return nullptr;
}