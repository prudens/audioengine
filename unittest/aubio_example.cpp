#include "aubio.h"
#include <string>
#include "webrtc\common_audio\wav_file.h"
#include "webrtc\common_audio\include\audio_util.h"
#include "onset\peakpicker.h"
void test_pitch(std::string filename,const char* method)
{
    FILE* data = fopen( "d:/log/pitch.txt", "w+");
    webrtc::WavReader reader_rec( filename );
    webrtc::WavWriter writer("d:/src/spc_noise_pro-16000.wav",reader_rec.sample_rate(),reader_rec.num_channels());
    // 1. allocate some memory
    uint_t n = 0; // frame counter
    uint_t win_s = 1024; // window size
    uint_t hop_s = win_s / 4; // hop size,256
    uint_t samplerate = reader_rec.sample_rate(); // samplerate
    // create some vectors
    fvec_t *input = new_fvec( hop_s ); // input buffer
    int16_t* src = new int16_t[hop_s];
    fvec_t *out = new_fvec( 1 ); // output candidates
    // create pitch object
    aubio_pitch_t *o = new_aubio_pitch(method, win_s, hop_s, samplerate );//mcomb yin yinfft schmitt fcomb specacf

    // 2. do something with it
    
    while ( n < 1000000 )
    {
        // get `hop_s` new samples into `input`
        size_t size = reader_rec.ReadSamples( hop_s, src );
        for ( int i = 0; i < input->length; i++ )
        {
            input->data[i] = webrtc::S16ToFloat( (int16_t)src[i] );
        }
        if (size < hop_s)
        {
            break;
        }
        // exectute pitch
        aubio_pitch_do( o, input, out );
        // do something with output candidates
        float d = out->data[0];
        fprintf( data, "%f\n", d );
        if ( d < 50000 )
        {
            if ( n % 10 == 0 )
            {
                printf( "\n" );
            }
            writer.WriteSamples(src,hop_s);
        }
        else
        {
            memset( src, 0, hop_s * 2 );
            writer.WriteSamples( src, hop_s );
            if ( n % 10 == 0 )
            {
                printf( "\n" );
            }
        }
        n++;
        printf( "[%d]%8.3f\t",n*256*10/(reader_rec.sample_rate()/100), d);



    };

    // 3. clean up memory
    del_aubio_pitch( o );
    del_fvec( out );
    del_fvec( input );
    aubio_cleanup();
    fclose( data );
    data = nullptr;
}

void test_peak()
{
    uint_t length = 1024;
    fvec_t* in = new_fvec( length );
    for ( uint_t i = 1; i < length+1;i++ )
    {
        float x = 3.14159 / length * i*10;
        
        in->data[i-1] = x * 10000;
    }

    fvec_t * out = new_fvec( 1 ); // input buffer
    aubio_peakpicker_t * o = new_aubio_peakpicker();
    aubio_peakpicker_set_threshold( o, 0.3 );
    aubio_peakpicker_do( o, in, out );
    aubio_peakpicker_do( o, in, out );
    aubio_peakpicker_do( o, in, out );
    aubio_peakpicker_do( o, in, out );

    del_aubio_peakpicker( o );
    del_fvec( out );
    del_fvec( in );
}



void test_aubio( int argc, char** argv )
{
    test_peak();
    return;
    printf( "\n" );
    char*method [] = { "mcomb", "yin", "yinfft", "schmitt", "fcomb", "specacf" };
    for ( int i = 0; i < 6;i++ )
    {
        printf( "%s\n", method[i] );
        test_pitch( "d:/src/music.wav", method[i] );
        printf( "\n" );
    }

    //test_pitch( "d:/pitch_audio_test/xiaojialiang.wav" );
}