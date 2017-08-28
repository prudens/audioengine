#include "header.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>
#include "base/async_task.h"
//#include "dispatch_example.cpp"

// Example: main calls myfunc
extern int test_vcl( int argc, char* argv[] );


void test_async_task()
{
    AsyncTask asyn_task(1);
    for ( auto i : {5,2,3,3,1} )
    asyn_task.AddTask( [] ( uint32_t id ) { printf( "%d  ", id ); },i );

    system( "pause" );
}

#include "webrtc/common_audio/real_fourier.h"
#include "base/time_cvt.hpp"
#include <unordered_map>
void test_fft()
{
    const int order = 512;
    using namespace webrtc;
    auto pfft = RealFourier::Create( lround(log10(order)) );
    float arr[order];
    for ( int i = 0; i < order; i++ )
    {
        arr[i] = (float)i;
    }
    std::complex<float> caar[order];

    Timer t;
    for ( int i = 0; i < 2000; i++ )
    {
        pfft->Forward( arr, caar );
        pfft->Inverse( caar, arr );
    }
    cout << "Ooura Implemented fft processed time: "<<t.elapsed() <<"ms"<< endl;

    for ( int i = 0; i < order; i++ )
    {
        caar[i] = (float)i;
    }
    t.reset();
    for ( int i = 0; i < 2000; i++ )
    {
        CFFT::Forward( caar, order );
        CFFT::Inverse( caar, order );
    }

    cout <<"LIBROW implemented fft processed time :"<< t.elapsed() << "ms"<<endl;
    t.reset();
}


#include "audio_gain_control.h"

void test_voice_scale()
{
    WavReader reader_rec( "D:/log/test-16000.wav" );
    int samplerate = reader_rec.SampleRate();
    int channel = reader_rec.NumChannels();
    WavWriter writer( "D:/log/test1.wav", samplerate, channel );

    AudioGainControl agc;

    const int size = 160;
    agc.Init( size, [&size] ( std::valarray< std::complex<float> >&frec, std::valarray<float>& amplitide, std::valarray<float>& angle ) {
        for ( int i = 0; i < size/8; i++ )
        {
            amplitide[i] *= 2;
        }
        return false; } );
    int16_t pPCMData[size] = { 0 };
    for ( ;; )
    {
        if ( 0 == reader_rec.ReadSamples( size, pPCMData ) )
        {
            break;
        }
        agc.ScaleVoice( pPCMData, size );
        writer.WriteSamples( pPCMData, size );
    }
}


void test_array();
void test_audio_processing(int argc,char**argv);
void test_codec();
void test_asio( int argc, char** argv );
void test_simulate_internet( int argc, char** argv );
void test_sound_to_text();
void test_audio_device();
void test_chrono();
#include <tuple>

void debug_out( const char* format, int argc, ... )
{
    
}

#define MACRO(format,...) debug_out(format,std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value, __VA_ARGS__ )
 
template <typename T>
T Argument( T value )
{
    return value;
}

template <typename ... Args>
void Print( char const * const format,
            Args const & ... args )
{
    std::cout<<sizeof...( args )<<std::endl;
    if ( sizeof...( args ) == 0 )
    {
        printf("%s",format);
    }
    else
    {
        char buf[512];// = { 0 };
        sprintf( buf, format, Argument( args ) ... );
        std::cout << buf << "\n";
    }

}
void test_rtp_rtcp_test( int argc, char** argv );
void test_numeric();

int main( int argc, char** argv )
{
  //  Print( "%s%s" );
   // test_asio(argc,argv);
  // test_chrono();
    test_numeric();
  //  test_codec();
   // test_audio_processing(argc,argv);
   // test_async_task();
  //  test_audio_device();
   // test_conv();
   // test_hrtf(45,0,"C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav","D:/pro-48000-1.wav");
   // test_real_time_3d();
   // test_mit_hrtf_get();
    //test_circular_buffer();
   // test_play_mp3();
   // test_vcl( argc, argv );
   // test_audio_ns();
   // test_audio_processing();


 //   test_aac_enc();
//    test_aac_dec();
//    test_aac_pasre_head();
 //   test_aac_dec_file();
//    run_mp32wav( "E:/CloudMusic/凡人歌.mp3" );

    //test_array();
   // test_fft();
    
   // test_voice_scale();

   // test_simulate_internet(argc,argv);
   // test_sound_to_text();

 //   test_rtp_rtcp_test( argc, argv );
    system( "pause" );
    return 0;

}