#include "io/mp3filereader.h"
#include "io/aacfilereader.h"
AudioReader* AudioReader::Create( const char*filename, AudioFileType type )
{
    if ( type == AFT_MP3 )
    {
        return new Mp3FileReader( filename );
    }
    else if ( type == AFT_AAC)
    {
        return new AACFileReader( filename );
    }
    return nullptr;
}