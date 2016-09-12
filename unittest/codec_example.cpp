#include "header.h"
#include "codec/aac/libAACenc/include/aacenc_lib.h"
#include "io/include/audioencoder.h"
void test_aac_enc()
{
    WavReader reader( "E:\\CloudMusic\\Mariage.wav" );
    int channel = reader.num_channels();
    int samplerate = reader.sample_rate();

    auto aacfile = AudioWriter::Create( "D:/Mariage.aac", samplerate, channel, AFT_AAC );
    int frame = 882;
    int16_t*buf = new int16_t[frame];
    for ( ;; )
    {
        auto len = reader.ReadSamples( 882, buf );
        if ( len > 0 )
        {
            aacfile->WriteSamples( buf, len );
        }
        else
        {
            break;
        }
    }

    aacfile->Destroy();
    delete[] buf;
}

#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
void test_aac_dec()
{
    WavWriter* writer = nullptr;// ( "D:/myvoice.wav" );
    FILE* file = fopen( "D:/myvoice.aac", "rb" );
    HANDLE_AACDECODER aacDecoderInfo = aacDecoder_Open( TT_MP4_ADTS, 1 );

    AAC_DECODER_ERROR err;
    err = aacDecoder_SetParam( aacDecoderInfo, AAC_DRC_REFERENCE_LEVEL, 1 );
    uint8_t* buf = new uint8_t[2048];
    UINT bufsize = 2048;
    INT_PCM* outbuf = new INT_PCM[4096];
    CStreamInfo* pStreamInfo = nullptr;
    UINT bytevalid = 0;
    for ( ;; )
    {
        if ( bytevalid == 0 )
        {
            bufsize = fread( buf, 1, 2048, file );
            if ( bufsize == 0 )
            {
                break;
            }
            bytevalid = bufsize;
        }
        UINT startpos = bufsize - bytevalid;
        bufsize = bytevalid;
        UCHAR*p = buf + startpos;
        err = aacDecoder_Fill( aacDecoderInfo, &p, &bufsize, &bytevalid );
        if ( err != AAC_DEC_OK )
        {
            printf( "aacDecoder_Fill() failed: 0x%x", err );
            return;
        }
        if ( bytevalid != 0 )
        {
            printf( "bytevalid = %d\n", bytevalid );
        }
        for ( ;; )
        {
            err = aacDecoder_DecodeFrame( aacDecoderInfo, outbuf, 4096, 0 );
            if ( err == AAC_DEC_NOT_ENOUGH_BITS )
            {
                break;
            }
            else if ( err != AAC_DEC_OK )
            {
                break;
            }
            if ( !pStreamInfo )
            {
                pStreamInfo = aacDecoder_GetStreamInfo( aacDecoderInfo );
                writer = new WavWriter( "D:/myvoice.wav", pStreamInfo->sampleRate, pStreamInfo->numChannels );
            }
            writer->WriteSamples( outbuf, pStreamInfo->frameSize*pStreamInfo->numChannels );
        }

    }
    aacDecoder_Close( aacDecoderInfo );
    if ( writer )delete writer;
    delete[] buf;
    delete[] outbuf;
}
struct AACDecHeader
{
    uint16_t Syncword;//12
    uint8_t version;//1
    uint8_t Layer;//2
    uint8_t ProtAbsent;//1
    uint8_t Profile;//2
    uint8_t sampleindex;//4
    uint8_t Private_Stream;//1
    uint8_t ChannelMode;//3;
    uint8_t Originality;//1
    uint8_t Home;//1
    uint8_t cr_Stream;//1
    uint8_t cr_Start;//1
    uint16_t frame_length;//13
    uint16_t buf_fullness;//11
    uint8_t num_frame;//2
    uint16_t CRC;//16
};
void test_aac_pasre_head()
{
    //‭111111111111 0 00 1 01 0111 0 010 0 0 0 0 0000010111001 00010101000 00‬
    FILE* file = fopen( "D:/myvoice.aac", "rb" );
    uint8_t header[7] = { 0 };
    AACDecHeader aacHeader;
    int index = 0;
    int seek = 0;
    for ( ;; )
    {
        if ( 7 != fread( header, 1, 7, file ) )
        {
            break;
        }
        uint8_t*p = header;
        aacHeader.Syncword = *p;
        p++;
        aacHeader.Syncword <<= 4;
        aacHeader.Syncword += ( *p >> 4 );
        aacHeader.version = ( *p >> 3 ) & 0x1;
        aacHeader.Layer = ( *p >> 1 ) & 0x3;
        aacHeader.ProtAbsent = *p & 0x1;
        p++;
        aacHeader.Profile = *p >> 6;//2
        aacHeader.sampleindex = ( *p >> 2 ) & 0x0f;//4
        aacHeader.Private_Stream = ( *p >> 1 ) & 0x1;
        aacHeader.ChannelMode = ( *p ) & 0x1;
        aacHeader.ChannelMode <<= 2;
        p++;
        aacHeader.ChannelMode += ( *p >> 6 ) & 0x3;//2
        aacHeader.Originality = ( *p >> 5 ) & 0x1;//1
        aacHeader.Home = ( *p >> 4 ) & 0x1;//1
        aacHeader.cr_Stream = ( *p >> 3 ) & 0x1;//1
        aacHeader.cr_Start = ( *p >> 2 ) & 0x1;//1
        aacHeader.frame_length = ( *p ) & 0x3;//2
        ++p;
        aacHeader.frame_length <<= 8;
        aacHeader.frame_length |= *p;
        aacHeader.frame_length <<= 3;
        ++p;
        aacHeader.frame_length |= *p >> 5;//3

        aacHeader.buf_fullness = *p & 0x1f;//5
        ++p;
        aacHeader.buf_fullness <<= 6;
        aacHeader.buf_fullness |= *p >> 2;//6
        aacHeader.num_frame = *p & 0x3;
        index++;
        printf( "[%d]frame_len = %d\t\t", index, aacHeader.frame_length );

        seek += aacHeader.frame_length;
        fseek( file, seek, SEEK_SET );
    }

    printf( "\n\n\n\n\n\n\nframe number=%d,sample_num=%d,time=%d", index, 4096 * ( index ), 4096 * ( index ) / 44100 / 2 );
    fclose( file );
}

#include "io/include/audioreader.h"
void test_aac_dec_file()
{
    AudioReader* pReader = AudioReader::Create( "D:/myvoice.aac", AFT_AAC );
    printf( "channel:%d", pReader->NumChannels() );
    printf( "samplerate:%d", pReader->SampleRate() );
    printf( "number sample:%d", pReader->NumSamples() );
    WavWriter writer( "D:/myvoice.wav", pReader->SampleRate(), pReader->NumChannels() );
    int16_t buf[4096];
    for ( int i = 0; i < 100; i++ )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    pReader->SeekSamples( pReader->NumSamples() / 2 );
    for ( ;; )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }

    pReader->Destroy();
}
#include <filesystem>

void run_mp32wav( const char* filename )
{
    using file_path = std::tr2::sys::path;

    AudioReader *mp3reader = AudioReader::Create( filename, AFT_MP3 );
    if ( mp3reader == nullptr )
    {
        return;
    }
    file_path p1 = filename;
    p1.replace_extension( ".wav" );

    WavWriter writer( p1.string().c_str(), mp3reader->SampleRate(), mp3reader->NumChannels() );
    int16_t buf[4096];
    for ( ;; )
    {
        int len = mp3reader->ReadSamples( 4096, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    mp3reader->Destroy();
}



class Mp3ReadProc : public  AudioBufferProc
{
    AudioReader* pMp3Reader;
    AudioWriter* pMp3Writer;
    int64_t   m_ts = 0;
public:
    Mp3ReadProc()
    {
        pMp3Reader = AudioReader::Create( "E:/CloudMusic/Mariage.mp3", AFT_MP3 );
        // pMp3Reader->SetSpeed( 1 );
        pMp3Writer = AudioWriter::Create( "D:/myvoice.aac", 44100, 2, AFT_AAC );
        std::cout << timestamp() << std::endl;
        m_ts = timestamp();
    }
    ~Mp3ReadProc()
    {
        auto ts = timestamp();
        std::cout << "\n record time: " << ts - m_ts << " sample number： " << pMp3Writer->NumSamples() / pMp3Writer->SampleRate() / pMp3Writer->NumChannels() << std::endl;
        pMp3Reader->Destroy();
        pMp3Writer->Destroy();
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte / 2 );
    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        int len = pMp3Reader->ReadSamples( size_in_byte / 2, (int16_t*)data );
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte / 2 );
        if ( len == 0 )
        {
            pMp3Reader->SeekTime( 0 );
        }
        return size_in_byte;
    }
};



void test_play_mp3()
{
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    Mp3ReadProc cb;
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    //  pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->StopPlayout();
    pWinDevice->StopRecording();
    pWinDevice->Terminate();
    pWinDevice->Release();
}

class G7221EncoderProc: public  AudioBufferProc
{
    AudioEncoder* encoder;
    FILE *hFile;
    char buf[2048];
    int maxLen = 2048;
    Resampler m_recResample;
    int m_channel;
    int16_t input[160];
public:
    G7221EncoderProc(const char* file,int samplerate,int channel)
    {
        m_channel = channel;
        hFile = fopen( file, "wb+" );
        encoder = AudioEncoder::Create( AFT_G7221 );
        m_recResample.Reset( samplerate ,16000,1);
    }
    ~G7221EncoderProc()
    {
        encoder->Release();
        fclose( hFile );
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        int16_t*pData = (int16_t*)data;
        if (m_channel == 2)
        {

            for ( size_t i = 0; i < size_in_byte/4; i++)
            {
                pData[i] = pData[i * 2];
            }
        }
        size_t outLen = 0;
       // m_recResample.Push( pData, size_in_byte / 2, input, 160, outLen );
        maxLen = 2048;
        encoder->Encode( (int16_t*)pData, 160*2, buf, maxLen );
        if (maxLen>0)
        {
            fwrite( buf, 1, maxLen, hFile );
        }

    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        return 0;
    }
};
void test_encoder_g7221()
{
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 16000, 2 );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    uint32_t samplerate;
    uint16_t channel;
    pWinDevice->GetRecordingFormat( samplerate, channel );
    G7221EncoderProc cb( "D:/rec.pak", samplerate, channel );
     pWinDevice->SetAudioBufferCallback( &cb );
    //pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->StopPlayout();
    pWinDevice->StopRecording();
    pWinDevice->Terminate();
    pWinDevice->Release();

}

void test_codec()
{
    test_encoder_g7221();
}