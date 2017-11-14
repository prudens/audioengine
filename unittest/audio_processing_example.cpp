#include "header.h"
#include "webrtc/modules/audio_processing/agc/legacy/gain_control.h"
#include "webrtc_agc.h"
#include "webrtc/modules/audio_processing/ns/noise_suppression.h"
#include "webrtc/common_audio/vad/include/webrtc_vad.h"
#include "webrtc/common_audio/resampler/include/push_resampler.h"
#include "audio_resample.h"
void WavFileProcess( std::string inFile, std::string outFile, size_t frametime = 10 /*ms*/, std::function<bool( int16_t*, size_t& )> fn = nullptr )
{
    assert( fn );
    if (!fn)
    {
        return;
    }
    if (inFile.empty())
    {
        return;
    }
    WavReader* reader = new WavReader(inFile);
    int channel = reader->NumChannels();
    int samplerate = reader->SampleRate();
    
    WavWriter* writer = new WavWriter(outFile,16000,1);
    AudioResample audio_resample;
    audio_resample.Reset( samplerate, channel, 16000, 1 );
    size_t frame_size = samplerate / 1000 * frametime * channel;
    int16_t* audio_frame = new int16_t[frame_size];
    int16_t* process_frame = new int16_t[16*frametime];
    bool run = true;
    for ( ;run; )
    {
        if ( reader->ReadSamples( frame_size, audio_frame ) != frame_size )
        {
            break;
        }

        size_t outSamples = 16 * frametime;
        audio_resample.Process( audio_frame, frame_size, process_frame, outSamples );
        assert( outSamples == 16*frametime );
        
        run = fn(process_frame, outSamples);
        if(writer)
            writer->WriteSamples( process_frame, outSamples );
    }
    delete process_frame;
    delete audio_frame;
    delete writer;
    delete reader;
}
void test_audio_effect(  )
{
    WavReader reader_rec( "d:/test/capture.wav" );


    int samplerate = reader_rec.SampleRate();
    int channel = reader_rec.NumChannels();
    WavWriter writer( "d:/test/capture-ens.wav", samplerate, channel );

    AudioEffect ae;
    ae.EnableAgc( false );
    ae.EnableVad( false );
    ae.EnableNs( true );
    size_t frames = samplerate / 100 * channel;
    int16_t* buf = new int16_t[frames];
    size_t byte_of_len = frames*2;
    int count = 0;
    for ( ;; )
    {
        if ( frames != reader_rec.ReadSamples( frames, buf ) )
            break;

        ae.ProcessCaptureStream( buf, frames * 2 );
        if (!ae.HasVoice())
        {
            memset( buf,0, byte_of_len );
            count++;
        }
        writer.WriteSamples( buf, byte_of_len/2 );
    }

    delete[] buf;
    printf( "count=%d", count );
} 

void test_webrtc_ns()
{
    WavReader reader( "d:/test.wav" );
    WavWriter writer( "d:/test-webrtc-ns.wav", reader.SampleRate(),reader.NumChannels() );
    std::vector<int16_t> srcFrame(reader.SampleRate()/100, 0);
    std::vector<float>   nsFrame(reader.SampleRate()/100,  0);
    std::vector<float>   nsFrameOut( reader.SampleRate() / 100, 0 );
    int err = 0;
    auto handle = WebRtcNs_Create();
    assert(0==WebRtcNs_Init( handle, reader.SampleRate() ));
    assert(0 == WebRtcNs_set_policy( handle, 3 ));
    for ( ;; )
    {
        if( srcFrame.size() != reader.ReadSamples( srcFrame.size(), srcFrame.data() ))
            break;
        S16ToFloat( srcFrame.data(), srcFrame.size(), nsFrame.data() );
        WebRtcNs_Analyze(handle , nsFrame.data());
        const char* src = (const char*)nsFrame.data();
        const char* dst = (const char*)nsFrameOut.data();
        WebRtcNs_Process( handle, (const float* const*)( &src ), 1, (float*const *)( &dst ) );
        float p = WebRtcNs_prior_speech_probability( handle );
        const float * v = WebRtcNs_noise_estimate( handle );
        int i = (int)(p * 10);
        printf( "%.2f  ", p );
        FloatToS16(nsFrameOut.data(),nsFrameOut.size(),srcFrame.data());
        writer.WriteSamples( srcFrame.data(), srcFrame.size() );
    }
    WebRtcNs_Free( handle );
}

void test_audio_ans()
{
    std::string inputfile;
   // inputfile = "d:/wavxiaomi4/wav/after_rtc.wav";
      inputfile = "d:/Users/zhangnaigan/Desktop/zng/731/wav/after_rtc.wav";
      inputfile = "d:/Users/zhangnaigan/Desktop/zng/noise/after_rtc.wav";
   // inputfile = "D:\\LinuxShared\\project\\trunk-dev\\build\\winx\\win32\\Debug\\app_data\\tester\\wav\\after_rtc.wav";
    inputfile = "d:\\Users\\zhangnaigan\\Desktop\\gaopeng\\728\\wav1\\wav\\after_rtc.wav";
    std::string outputfile = inputfile + ".process.wav";
    using namespace snail::audio;
    AudioNoiseSuppression ans;
    ans.Init( 16000, 1 );

    VadInst* handle = WebRtcVad_Create();
    WebRtcVad_Init( handle );
    WebRtcVad_set_mode( handle, 1 );

     
    if ( 0 != WebRtcVad_ValidRateAndFrameLength( 16000, 160 ) )
    {
        return;
    }

    WavFileProcess( inputfile, outputfile, 10, [&] (int16_t*data, size_t& nSample) 
    {
        int activity = WebRtcVad_Process( handle, 16000, data, nSample );
        auto pFrame = ans.Proceess( data, nSample, activity == 0 );
        if (pFrame)
        {
            memcpy( data, pFrame->data, pFrame->size * 2 );
        }
        else
        {
            memset( data, 0, nSample * 2 );
        }
        return true;
    } );
    WebRtcVad_Free( handle );
}
#include "audio_normalized.h"
void test_audio_normalized()
{
    std::string inputfile;
    // inputfile = "d:/wavxiaomi4/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/731/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/noise/after_rtc.wav";
    // inputfile = "D:\\LinuxShared\\project\\trunk-dev\\build\\winx\\win32\\Debug\\app_data\\tester\\wav\\after_rtc.wav";
    inputfile = "d:\\Users\\zhangnaigan\\Desktop\\gaopeng\\728\\wav\\capture.wav";
    inputfile = "D:\\Users\\zhangnaigan\\Documents\\Tencent Files\\1058068847\\FileRecv\\MobileFile\\capture(1).wav";
    std::string outputfile = inputfile + ".process.wav";
    using namespace snail::audio;
    AudioNormalized an;
    an.SetLevel( 70 );
    int count = 0;

    VadInst* handle = WebRtcVad_Create();
    WebRtcVad_Init( handle );
    WebRtcVad_set_mode( handle, 1 );
    WebRtcVad_ValidRateAndFrameLength( 16000, 160 );

    WavFileProcess( inputfile, outputfile, 10, [&] ( int16_t*data, size_t& nSample )
    {
        an.Process( data, nSample );
        int activity = WebRtcVad_Process( handle, 16000, data, nSample );
        an.Update( activity == 0);

        return true;
    } );
}
#include "webrtc\\modules\\audio_processing\\rms_level.h"
void test_audio_reduce_noise_by_RMS()
{

    WavReader reader( "d:/3.wav" );
    int samplerate = reader.SampleRate();
    int channel = reader.NumChannels();
    WavWriter writer( "d:/5.wav", samplerate, channel );
    int frame_size = samplerate / 100 * channel;
    
    RMSLevel rms;
    std::list<int> before_levels;
    std::list<int> after_levels;
    std::list<int16_t*> after_buf;
    const int N = 5;
    for ( ;; )
    {
        int16_t* buffer = new int16_t[frame_size];
        if ( frame_size != reader.ReadSamples( frame_size, buffer ) )
        {
            break;
        }
        rms.Process( buffer, frame_size );
        int level = rms.RMS(); 
        //printf( "%d ", level );
        for ( ;; )
        {
            if ( before_levels.size() < N )
            {
                before_levels.push_back( level );
                break;
            }

           
            //ÑÓÊ±¡£
           after_levels.push_back( level );
           after_buf.push_back( buffer );
           if (after_levels.size()< N)
           {
               break;
           }


            if ( before_levels.size() > N )
            {
                before_levels.pop_front();
            }

            level = after_levels.front();
            buffer = after_buf.front();
            auto it = after_levels.begin();
            it++;
            int before_avg = std::accumulate( before_levels.begin(), before_levels.end(), 0 ) / before_levels.size();
            int after_avg = std::accumulate( it, after_levels.end(), 0 ) / (after_levels.size()-1);
            float scale = 1.0f;
            if ( before_avg - level > 5 /*&& after_avg - level > 6*/ )
            {
                scale = pow( 10.0f, (float)( level -  before_avg ) / 20.0f );
                level = before_avg;
            }
            if ( scale < 1 )
            {
//                 int16_t* p = (int16_t*)buffer;
//                 for ( int j = 0; j < frame_size; j++ )
//                 {
//                     p[j] = p[j] * scale;
//                 }
                memset(buffer,0,frame_size*2);
            }

            before_levels.push_back( level );
            if (after_levels.size() > N)
            {
                after_levels.pop_front();
                after_buf.pop_front();
            }

            break;
        }
        writer.WriteSamples( buffer, frame_size );
        printf( "%d ", level );
        delete buffer;
        buffer = nullptr;
    }




}

void test_webrtc_agc()
{
    WavReader reader( "d:\\Users\\zhangnaigan\\Desktop\\111.xcappdata\\AppData\\Documents\\tester\\wav\\after_rtc.wav" );
    WavWriter writer("d:/test/after_rtc-agc-L6.wav",reader.SampleRate(),reader.NumChannels());

   int kFrames = reader.SampleRate()/100*reader.NumChannels();
   int16_t *inBuf = new int16_t[kFrames];

   WebrtcAgc agc;
   agc.Init(reader.SampleRate(),reader.NumChannels());
   agc.SetTargetLevel( 6 );
   while (true)
   {
       if ( kFrames != reader.ReadSamples( kFrames, inBuf ) )
       {
           break;
       }

       agc.Process( inBuf, kFrames );
       writer.WriteSamples( inBuf, kFrames );
   }
   delete[] inBuf;
}

void test_webrtc_vad()
{
    int count = 0;
    VadInst* handle = WebRtcVad_Create();
    WebRtcVad_Init( handle );
    WebRtcVad_set_mode( handle, 1 );
    WavReader reader("d:/wavxiaomi4/wav/after_rtc.wav");
    int channel = reader.NumChannels();
    int samplerate = reader.SampleRate();
    WavWriter writer( "d:/wavxiaomi4/wav/after_rtc-vad.wav", samplerate, channel );

    int N = samplerate / 100;
    int16_t* frame = new int16_t[N];
    if ( 0 != WebRtcVad_ValidRateAndFrameLength( samplerate, N ) )
    {
        return;
    }
    for ( ;; )
    {
        if ( reader.ReadSamples(N,frame) != N)
        {
            break;
        }
        int activity = WebRtcVad_Process( handle, samplerate, frame, N );
        if (activity == 0)
        {
            memset( frame, 0, N * 2 );
            count++;
        }
        writer.WriteSamples( frame, N );
    }
    WebRtcVad_Free( handle );
    printf( "count=%d",count );
}

#include "webrtc\\modules\\audio_processing\\vad\\vad_audio_proc.h"
void test_webrtc_audio_feature()
{
    VadAudioProc audio_proc;
    AudioFeatures feature;
    int count=0;
    WavFileProcess( "d:/test/test1.wav", "d:/test/test2.wav", 10, [&](int16_t* audio_frame, size_t& frame_size)
    {

        memset( &feature, 0, sizeof( feature ) );
        audio_proc.ExtractFeatures( audio_frame, 160, &feature );
        for ( int i = feature.num_frames-1; i >= 0; i-- )
        {
            printf( "[%d]%.3f %.3f \n", count, feature.pitch_lag_hz[feature.num_frames - i - 1], feature.spectral_peak[feature.num_frames - i - 1] );
            count++;
        }

        return true;
    } );

}


#include "processing/src/audio_mixer/include/audio_mixer.h"
class AudioStream :public MixerParticipant
{
public:
    AudioStream( const char* file )
    {
        reader = new WavReader( file );
        channel = reader->NumChannels();
        samplerate = reader->SampleRate();
    }
    virtual size_t GetAudioFrame( int16_t* data, size_t nSample )
    {
        return reader->ReadSamples( nSample, data );
    }
    ~AudioStream()
    {
        delete reader;
    }
private:
    WavReader*reader;
    int samplerate;
    int channel;
};

void test_audio_mixer()
{
//    WavWriter output( "D:/es-output.wav",48000,2 );
//    AudioStream es01("C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav");
//    AudioStream es02( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/es02.wav" );
//    AudioStream es03( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/es03.wav" );
//    AudioStream es04( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc01.wav" );
//    AudioStream es05( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc02.wav" );
//    AudioStream es06( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc03.wav" );
//    AudioMixer* mixer = AudioMixer::Create( 48000, 2, 960 );
//    mixer->LimitParticipantCount( 1 );
//    mixer->AddParticipant( &es01 );
//    mixer->AddParticipant( &es02 );
////  mixer->AddParticipant( &es03 );
////  mixer->AddParticipant( &es04 );
//    mixer->AddParticipant( &es05 );
////  mixer->AddParticipant( &es06 );
//
//    int16_t data[960];
//    for ( ;; )
//    {
//        size_t nsample = mixer->GetMixerAudio( data, 960 );
//        if (nsample > 0)
//        {
//            output.WriteSamples( data, nsample );
//        }
//        else
//        {
//            break;
//        }
//    }

}

#include "mmse.h"
#include "base/time_cvt.hpp"
void test_mmse()
{

    WavReader reader( "d:/zaosheng-ns.wav" );
    int samplerate = reader.SampleRate();
    int channel = reader.NumChannels();
    WavWriter writer( "d:/zaosheng-ns-mmse-1.wav", samplerate, channel );
    MMSE mmse;
    mmse.Init( samplerate, channel );
    const int kFrameSize = 160;
    int16_t buf[kFrameSize] = { 0 };
    
    while (true)
    {
        if ( kFrameSize != reader.ReadSamples( kFrameSize, buf ) )
        {
            break;
        }
        mmse.Process( buf, kFrameSize );
        writer.WriteSamples( buf, kFrameSize );
    }
}

#include "findpeaks.h"
void test_findpeaks( )
{
    float a[] = { 0.0043,0.0172,0.1257};
 //  float a[] = { 1, 2, 3, 2, 1, 2, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 4, 1, 4 };
    //float a[]= { 1.f, 2.f, 1.f, 4.f, 5.f,1.0f };
    FindPeaks find_peaks;
    //find_peaks.Filter(a,5);
    find_peaks.SetMinPeakDistance( 5 );
    find_peaks.SetMinPeakHeight( 0.0f );
   auto res = find_peaks.Process( a, sizeof(a)/sizeof(a[0]) );
   for ( size_t i = 0; i < res.size(); i++ )
   {
       printf( "[%d] %.3f\n", res[i].second+1,res[i].first);
   }
}
#include "aec_detect.h"
void test_aec_detect()
{
    std::string inputfile;
    // inputfile = "d:/wavxiaomi4/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/731/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/noise/after_rtc.wav";
    // inputfile = "D:\\LinuxShared\\project\\trunk-dev\\build\\winx\\win32\\Debug\\app_data\\tester\\wav\\after_rtc.wav";
    //inputfile = "d:\\Users\\zhangnaigan\\Desktop\\gaopeng\\728\\wav1\\wav\\after_rtc.wav";
    inputfile = "d:\\Users\\zhangnaigan\\Desktop\\mixer\\wav-xjl\\wav\\capture.wav";
    inputfile = "D:\\Users\\zhangnaigan\\Documents\\Tencent Files\\1058068847\\FileRecv\\MobileFile\\after_rtc(2).wav";
    std::string outputfile = inputfile + ".process.wav";

    AecDetect ad;
    WavFileProcess( inputfile, outputfile, 10, [&] ( int16_t*data, size_t& nSample )
    {
        ad.ProcessCaptureAudio( data, nSample );
        return true;
    } );
//     inputfile = "d:\\Users\\zhangnaigan\\Desktop\\mixer\\wav-xjl\\wav\\playout.wav";
//     WavFileProcess( inputfile, outputfile, 10, [&] ( int16_t*data, size_t& nSample )
//     {
//         ad.ProcessRenderAudio( data, nSample );
//         return true;
//     } );
// 
//     ad.IsNeedWebrtcAec();
}
#include "audio_voice_check_base.h"
void test_voice_check()
{
    std::string inputfile;
    // inputfile = "d:/wavxiaomi4/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/731/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/noise/after_rtc.wav";
    // inputfile = "D:\\LinuxShared\\project\\trunk-dev\\build\\winx\\win32\\Debug\\app_data\\tester\\wav\\after_rtc.wav";
    //inputfile = "d:\\Users\\zhangnaigan\\Desktop\\gaopeng\\728\\wav1\\wav\\after_rtc.wav";
    inputfile = "d:\\Users\\zhangnaigan\\Desktop\\mixer\\wav-xjl\\wav\\capture.wav";
    inputfile = "D:\\Users\\zhangnaigan\\Documents\\Tencent Files\\1058068847\\FileRecv\\MobileFile\\after_rtc(2).wav";
    std::string outputfile = inputfile + ".process.wav";

    using namespace snail::audio;
    AudioVoiceCheck check;
    check.Reset( 10 );
    check.SetLevel( 40 );

    VadInst* handle = WebRtcVad_Create();
    WebRtcVad_Init( handle );
    WebRtcVad_set_mode( handle, 3 );
    WebRtcVad_ValidRateAndFrameLength( 16000, 160 );

    webrtc::RMSLevel rms;
    WavWriter* writer = new WavWriter( "d:/808.wav", 16000, 1 );
    WavFileProcess( inputfile, outputfile, 10, [&] ( int16_t*data, size_t& nSample )
    {
        int activity = WebRtcVad_Process( handle, 16000, data, nSample );
        rms.Process( data, nSample );
        int level = rms.RMS();
        check.Process( data, nSample * 2, activity, level);
        for ( ;; )
        {
            auto p = check.GetFrame();
            if (!p)
            {
                break;
            }
            writer->WriteSamples( (int16_t*)p->data(), p->len / 2 );
        }
        return true;
    } );
    delete writer;
    WebRtcVad_Free( handle );
}

//b = [1,2,1];
//a=[1,2];
//   a( 1 )*y( n ) = b( 1 )*x( n ) + b( 2 )*x( n - 1 ) + ... + b( nb + 1 )*x( n - nb )
// -a( 2 )*y( n - 1 ) - ... - a( na + 1 )*y( n - na )
void filter( int16_t* x, size_t length, float *b, float*a, size_t Nb = 3, size_t Na = 2 )
{
    int16_t* y = new int16_t[length];
    memset( y, 0, length * 2 );
    for ( size_t n = 0; n < length; n++ )
    {
        float sb = 0.f;
        for ( size_t j = 0; j < Nb && j <= n; j++ )
        {
            sb += b[j] * x[n - j];
        }
        float sa = 0.f;
        for ( size_t j = 1; j < Na && j <= n; j++ )
        {
            sa += a[j] * y[n - j];
        }

        y[n] = std::round( (1.0f / a[0] )*( sb - sa ) );
    }
    memcpy( x, y, length * 2 );
    delete y;
}

class Filter
{
public:
    Filter( std::vector<float> b,std::vector<float> a, size_t frame_size )
    {
        _b = b;
        _a = a;
        _y.resize( frame_size + a.size() -1 );
        _x.resize( b.size() - 1 );

    }

    void Process( int16_t* x, size_t length)
    {
        size_t Nb = _b.size();
        size_t Na = _a.size();
        size_t state_n = Na - 1;
        for ( size_t n = 0; n < length; n++ )
        {
            float sb = 0.f;
            for ( size_t j = 0; j < Nb; j++ )
            {
                if ( n >= j)
                {
                    sb += _b[j] * x[n - j];
                }
                else
                {
                    sb += _b[j] * _x[Nb - 1 + n - j];
                }

            }
            float sa = 0.f;
            for ( size_t j = 1; j < Na; j++ )
            {
                sa += _a[j] * _y[n + state_n- j];
            }

            _y[n + state_n] = std::lround( ( 1.0f / _a[0] )*( sb - sa ) );
        }
        memcpy( &_x[0], &x[length - _x.size()], _x.size()*2 );
        memcpy( x, _y.data() + state_n, length * 2 );
        
        for ( size_t i = 0; i < state_n; i++ )
        {
            _y[i] = _y[_y.size() - i - state_n];
        }

    }
private:
    std::vector<float> _b;
    std::vector<float> _a;
    std::vector<int16_t> _y;
    std::vector<int16_t> _x;
};


void test_filter()
{
    std::string inputfile;
    // inputfile = "d:/wavxiaomi4/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/731/wav/after_rtc.wav";
    inputfile = "d:/Users/zhangnaigan/Desktop/zng/noise/after_rtc.wav";
    // inputfile = "D:\\LinuxShared\\project\\trunk-dev\\build\\winx\\win32\\Debug\\app_data\\tester\\wav\\after_rtc.wav";
    inputfile = "d:\\Users\\zhangnaigan\\Desktop\\gaopeng\\728\\wav1\\wav\\after_rtc.wav";
    //inputfile = "d:\\Users\\zhangnaigan\\Desktop\\xjl\\capture.wav";
   // inputfile = "D:\\Users\\zhangnaigan\\Documents\\Tencent Files\\1058068847\\FileRecv\\MobileFile\\after_rtc(2).wav";
    std::string outputfile = inputfile + ".process.wav";
    std::vector<float> b = { 1.f, 2.f, 1.f };
    std::vector<float> a = { 3.4f, 1.4f };
    Filter flt( b,a, 160 );
//     int16_t x[3] = { 100, 200, 300 };
//     flt.Process( x, 3 );
//     for ( int i = 0; i < 3; i++ )
//     {
//         printf( "%d ", x[i] );
//     }
//     for ( int i = 0; i < 3; i++ )
//     {
//         x[i] = (i+4)*100;
//     }
//     flt.Process( x, 3 );
//     for ( int i = 0; i < 3; i++ )
//     {
//         printf( "%d ", x[i] );
//     }
// 
//     return;
    WavFileProcess( inputfile, outputfile, 10, [&] ( int16_t*data, size_t& nSample )
    {
        flt.Process( data, nSample );
        return true;
    } );

}

void test_webrtc_resampler()
{
	std::string infile = "d:/xiangwang.wav"; //"D:\\Users\\zhangnaigan\\Downloads\\TestSignals\\Swept_24.wav";
	std::string outfile = infile + ".pro.wav";
	
	WavReader reader( infile );
	WavWriter writer( outfile, 44100, reader.NumChannels() );
	size_t length = reader.NumSamples();
	//PushResampler<int16_t> resampler;
	//resampler.InitializeIfNeeded(reader.SampleRate(),44100, reader.NumChannels() );
	AudioResample resampler;
	resampler.Reset( reader.SampleRate(), reader.NumChannels(), 44100, reader.NumChannels() );
	size_t block_size = reader.SampleRate() * reader.NumChannels() / 100; //10ms
	int16_t* inbuf = new int16_t[block_size];
	int16_t* outbuf=new int16_t[2048];
	for(size_t i = 0;; i++)
	{
		int read = reader.ReadSamples( block_size, inbuf );
		if(block_size != read )
		{
			break;
		}
		i += block_size;
		//int outlen = resampler.Resample( inbuf, block_size, outbuf, 2048 );
		int outlen = resampler.Process( inbuf, block_size, outbuf, 2048 );
		assert(outlen == 441*reader.NumChannels());
		writer.WriteSamples( outbuf, outlen );
	}
}

void test_audio_processing(int argc,char** argv)
{
   //test_audio_ans();
//     test_audio_effect();
// test_audio_reduce_noise_by_RMS();
// test_audio_mixer();
//   test_webrtc_agc();
// test_webrtc_vad();
 //test_audio_effect();
// test_mmse();
// test_webrtc_ns();
// test_webrtc_audio_feature();

//    test_findpeaks();
//    test_audio_normalized();
//    test_audio_mixer();
//    test_aec_detect();
//    test_voice_check();
//    test_filter();
	test_webrtc_resampler();
}