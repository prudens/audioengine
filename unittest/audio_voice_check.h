#pragma once
#include <list>
#include <cstdint>
#include "audio_low_pass_filter.h"
namespace snail{
	namespace audio{
		 
		class audio_voice_check
		{
		public: 
			struct tFrame
			{
				int   dbg_seq;
				int   len;
				int   type; 
				bool  vadspeaking;
				int   esclevel;
				void* data(){ return (char*)this + sizeof(tFrame); }
			};

		protected:
			struct tLevelStatic
			{
				int    levelsum;
				int    levelcount;
				int    levellen;
			};

			struct tAgc
			{  
				bool         enabled; 
				tLevelStatic once;
				tLevelStatic total;
				int          real_overflowcount;
				int		     overflowcount; 
				float	     scale; 
			};
			 

			typedef std::list<tFrame*> FrList; 
			struct tCache
			{ 
				bool   bSignal;
				int    endnospeaklen;
				int    spkcounter;
				int    allen;
				int    spk_level_len;
				FrList list;
				enum{findspk, speaking, spkending} staus;
				int    dbg_seqcounter;
			};

            struct LevelInfo
            {
                int level;
                bool isSpeech;
            };
            struct tnoise
            {
                std::list<LevelInfo> levels;
                float      voice_quality;
                int        noise_count;
                int        voice_count;
                tnoise() :voice_count( 0 ), noise_count( 0 ), voice_quality( 0.f ) {}
            };
		public:
			audio_voice_check();
			~audio_voice_check();

			void reset(int samplerate, int nchannel); 
			void process(const void* buf, int len, int esclevel, bool vadspk);
            void enable(bool bEnable);
			void set_level(int level);  
			bool isenable(){ return m_enable; } 
			tFrame* getframe(bool bclear);
			void freeFrame(tFrame* pf);

			void debug_level_analysis(int level); 

			void update_level(int level,bool isSpeech);
			void noise_suppression(void* data, int len, int index );
		protected:
			void volume_ctrl(void* buf, int len);
			void volume_check(void* buf, int len);
			void trace_max_level(const void* buf, int len);
			bool vad_check(const void* buf, int len);
			void agc_data_syn();
			void agc_computer();
			void cache_frame(const void* buf, int len, bool vadspeaking, int esclevel);
			void clear_cache(); 
		private:
			tCache     m_cache;  
			int		   m_10ms_len; 
			int        m_level_ttl;         // 小于此值，认为达到讲话能量
			int        m_level_ttl_nospeak; // 小于此值，一定认为不讲话
			bool       m_enable;   
			tAgc       m_agc;
			int        m_pref_vol;
			int        m_pref_ttl;
			int        m_spknumber;        // 讲话人数
			tnoise     m_noise;
            LowPassFilter m_lpf;
		};
	}
}
 