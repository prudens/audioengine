#include "io/mp3filereader.h"
#include "io/aacfilereader.h"
AudioReader* AudioReader::Create( const char*filename, AudioFileType type )
{
    if ( type == AFT_MP3 )
    {
        auto pReader = new Mp3FileReader( filename );
        if (!pReader->Initialized())
        {
            pReader->Destroy();
            return nullptr;
        }
        return pReader;
    }
    else if ( type == AFT_AAC)
    {
        auto pReader = new AACFileReader( filename );
        if ( !pReader->Initialized() )
        {
            pReader->Destroy();
            return nullptr;
        }
        return pReader;
    }
    return nullptr;
}