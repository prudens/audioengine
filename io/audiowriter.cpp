#include "io/include/audiowriter.h"
#include "aacfilewriter.h"
#include "mp3filewriter.h"
AudioWriter*AudioWriter::Create( const char* filename, int sample_rate, size_t channels, AudioFileType type )
{
    if ( type == AFT_MP3 )
    {
        auto pWriter = new Mp3FileWriter( filename, sample_rate, channels );
        if ( !pWriter->Initialized() )
        {
            pWriter->Destroy();
            return nullptr;
        }
        return pWriter;
    }
    else if ( type == AFT_AAC )
    {
        auto pWriter = new AACFileWriter( filename, sample_rate, channels );
        if ( !pWriter->Initialized() )
        {
            pWriter->Destroy();
            return nullptr;
        }
        return pWriter;
    }
    return nullptr;
}