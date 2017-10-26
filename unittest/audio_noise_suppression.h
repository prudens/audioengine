#pragma once
#include <list>
#include <cstdint>
#include <complex>
#include "webrtc/common_audio/real_fourier.h"
#include "webrtc/modules/audio_processing/rms_level.h"
#include "findpeaks.h"
// 延时50ms左右

#include "fft_wrapper.h"
#include "webrtc_agc.h"
#include "audio_parse_param.h"
namespace snail{
    namespace audio{
        struct SignalStat
        {
            bool lowSignal = true;
            std::list<float> silent_levels;
            std::list<size_t> speech_levels;
            int hilevelcount = 0;
            int LowerSourceTTL=20;
            int speech_diff_level = 0;
            float coef = 0.01f;
            float min_height = 0.0f;
        };
        class AudioNoiseSuppression :public IParseParamNotify
        {
            typedef std::complex<float> Complex;
        public:
            enum NoiseType
            {
                Speech,       // 正常语音
                Consonant,    // 辅音
                FullHighFreq, // 无低频区域 
                Noise,        // 噪音
                Silent,       // 静音
            };
            struct VoiceFrame
            {
                int16_t data[480 * 2]; // 数据帧内容
                int size;            // 当前帧长度
                int level;           // 能量等级，越大表示能量越小
                int silent;          // 是否是静音 
            };


            struct tProcess
            {
                int NoSpeechCnt = 0;
                int NoiseCnt = 0;
                int ConsonantCnt = 0;
                int SilentCnt = 0;
            };

            struct NoiseCoef
            {
                float std;             // 峰值的离散度
                float max_freq;        // 语音最大频率分量
                float low_level;       // 判断是否低音区
                float silent_level;    // 静音平均能量
                float min_level;       // 最小语音帧的能量
                int   num_peak;        // 当前帧有多少峰值点
                int   peak_height;     // 寻找峰值的最低高度
                int   peak_distance;   // 寻找峰值的最小距离
            };

            AudioNoiseSuppression();
            ~AudioNoiseSuppression();
            void Init( int samplerate, int nChannel );
            VoiceFrame* Proceess( int16_t* data, size_t len, bool silent );
            void Reset();
            void enable( bool bEnable ) { m_bEnable = bEnable; }
            virtual void ParseParamNotify( const std::string& Param );
            bool ProcessSpec( std::valarray< std::complex<float> >&frec, std::valarray<float>& amplitide, std::valarray<float>& angle );
            bool ProcessMMSE( std::valarray<float>& sig );
            VoiceFrame* ProceessNS( int16_t* data, size_t size, int level );
            int AnalyzePeaks( std::valarray<float>& amplitide );
            void ProcessLowLevelSignal( int16_t* data, size_t len );
            float Std(const std::vector<std::pair<float,size_t>>& pks);
        private:
            std::list<int> m_future_list;
            std::list<VoiceFrame*> audio_cache;
            std::list<VoiceFrame*> audio_cache_free;
            WebrtcAgc m_agc;
            webrtc::RMSLevel m_rms;

            int m_recSamplerate = 16000;
            int m_recChannel = 1;
            int m_frame;


            tProcess m_process;


            double m_silent_Threshold = 0.01;
            bool  m_bEnable = true;
            bool  m_disableForRecordingMsg = false;


            FFTWrapper m_fft;
            std::valarray<float> m_noise;
            std::valarray<float> m_Xk_prev;
            int m_init_num = 0;

            bool m_bsilent = false;
            int m_noise_type = Speech;
            bool m_bEnableMMSE = false;
            float *m_energy = nullptr;
            FindPeaks m_find_peaks;
            int m_audio_type_list[24];
            FILE*m_file=nullptr;
            NoiseCoef m_noise_coef;
        };

    }
}
