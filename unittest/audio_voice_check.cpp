#include "audio_voice_check.h"
#include <map>
#include <windows.h>
 
#define  log_out_cat 
#define logcat_set(...) printf("");
#define logcat(...)   printf("");
#define tickcount GetTickCount
#define tick_t double
namespace snail{
	namespace audio{

		#define FSC2SC(x)       (int16_t)(x*32)
		#define NOSCALE_VAL     32
		#define SC_RATE		    5
		#define SCALRE(x, sc)   (sc*(int32_t)(x))>>SC_RATE
        #define MAX_VOL_SC      8.0f 
		#define PREF_TTL_       25
        #define PREF_MAXVOL_    18000

		//cs[e:1,l:40]
		//gv[t:25,v:18000,n:5]

		audio_voice_check::audio_voice_check()
		{ 
			logcat_set(true); 
			//snail::kernel::CSystem::GetLogger()->set_config(level_trace, snail::kernel::log_debug);
			 

			m_pref_vol  = PREF_MAXVOL_;
			m_pref_ttl  = PREF_TTL_;
			m_spknumber = 5;

			int32_t value = 0;
			   
			m_enable = true;


			
			set_level(40);

			 
			m_10ms_len      = 0;  
			memset(&m_agc, 0, sizeof(tAgc));
			m_agc.scale = 4.0f; 
			m_agc.enabled = true;
			memset(&m_agc.once, 0, sizeof(tLevelStatic));
			memset(&m_agc.total, 0, sizeof(tLevelStatic));

			m_agc.enabled = true;

			m_cache.allen = 0;
			m_cache.spk_level_len = 0;
			m_cache.spkcounter = 0; 
			m_cache.dbg_seqcounter = 0; 
		}


		audio_voice_check::~audio_voice_check()
		{
			clear_cache();
		}

		void audio_voice_check::reset(int samplerate, int nchannel)
		{
			m_10ms_len = samplerate * 2 * nchannel * 10 / 1000;   
			clear_cache();
			memset(&m_agc.once, 0, sizeof(tLevelStatic));
			memset(&m_agc.total, 0, sizeof(tLevelStatic));
		}

        void audio_voice_check::enable(bool bEnable)
        {
            m_enable = bEnable;
        }

		 
        void audio_voice_check::set_level( int level )
        {
			if (level <= 10 || level > 80)
				return;
			
			m_level_ttl = level; 

			if (m_level_ttl <= 32)
				m_level_ttl_nospeak = 42;
			else if (m_level_ttl <= 42)
				m_level_ttl_nospeak = 52;
			else if (m_level_ttl <= 52)
				m_level_ttl_nospeak = 62;
			else if (m_level_ttl <= 62)
				m_level_ttl_nospeak = 80;
			else
				m_level_ttl = 100;
        }

		bool audio_voice_check::vad_check(const void* buf, int len)
		{
			int16_t* pSamples = (int16_t*)buf;
			int       samples = len / 2;

			int       spksamples = 0; 
			for (int index = 0; index < samples; index++)
			{
				int16_t sample = *(pSamples + index); 

				if (sample > m_level_ttl || sample < -m_level_ttl)
				{
					spksamples++;
					if (spksamples > 20)
						return true;
				}
			}

			return false;  
		}

		void audio_voice_check::volume_check(void* buf, int len)
		{
			if (!m_agc.enabled)
				return;
			 
			int overflowcount = 0;
			int overflowsum = 0;

			int16_t sc = FSC2SC(m_agc.scale);
			if (NOSCALE_VAL == sc)
				return;

			int16_t*  wav   = (int16_t*)buf;
			int32_t samples = len >> 1; 

			for (int index = 0; index < samples; index++)
			{
				int sample = *wav*m_agc.scale;// SCALRE(*wav, sc);

				if (sample < -m_pref_vol)
				{
					overflowcount++;
					overflowsum += -sample / 100; 
				}
				else if (sample > m_pref_vol)
				{
					overflowcount++;
					overflowsum += sample / 100; 
				} 

				wav++;
			}

			if (overflowcount > 0)
			{ 
				int avgoverflow = overflowsum / overflowcount;

				m_agc.scale = m_agc.scale * m_pref_vol / 100 / avgoverflow;
				if (m_agc.scale < 0.8f)
					m_agc.scale = 0.8f;  
			} 
		}
		 
		void audio_voice_check::volume_ctrl(void* buf, int len)
		{   
			if (!m_agc.enabled)
				return;
			  
			int16_t sc = FSC2SC(m_agc.scale);
			if (NOSCALE_VAL == sc)
				return;

			int16_t*  wav = (int16_t*)buf;
			int32_t samples = len >> 1;
			 
			for (int index = 0; index < samples; index++)
			{
				int sample = *wav*m_agc.scale; //SCALRE(*wav, sc);

				if (sample < -m_pref_vol)
				{
					m_agc.overflowcount++; 
				}
				else if (sample > m_pref_vol)
				{
					m_agc.overflowcount++; 
				}

				if (sample < -32767)
				{ 
					*wav = -32767;
					m_agc.real_overflowcount ++;
				}					
				else if (sample > 32767)
				{  
					*wav = 32767;
					m_agc.real_overflowcount++;
				}
				else
				{
					*wav = sample;
				} 

				wav++;
			}  
		}

		void audio_voice_check::agc_data_syn()
		{ 
			if (0 == m_agc.total.levellen)
			{ 
				m_agc.total.levelsum = m_agc.once.levelsum;
				m_agc.total.levelcount = m_agc.once.levelcount;
				m_agc.total.levellen = m_agc.once.levellen; 
			}
			else
			{
				if (m_agc.total.levellen < 100*1024 * 1024)
				{
					m_agc.total.levelsum += m_agc.once.levelsum;
					m_agc.total.levelcount += m_agc.once.levelcount;
					m_agc.total.levellen += m_agc.once.levellen; 
				} 
			}

			logcat("---- speaking level len: %d, %d\n", 10*m_agc.once.levellen / m_10ms_len, 10*m_agc.total.levellen / m_10ms_len);
			 
			memset(&m_agc.once, 0, sizeof(tLevelStatic));  
		}

		void audio_voice_check::agc_computer()
		{ 
			if (m_agc.total.levellen < 20 * m_10ms_len) 
				return; 

			float oldsc  = m_agc.scale;
			int avglevel = m_agc.total.levelsum / m_agc.total.levelcount;
			int dttl = 10 * log10(m_agc.scale*m_agc.scale);

			int currentlevel = avglevel - dttl;

			if (currentlevel > m_pref_ttl)
			{
				if (m_agc.scale < MAX_VOL_SC && m_agc.overflowcount < 100)
				{
					float prefsc = sqrt(pow(10, (float)(avglevel - m_pref_ttl) / 10));
					m_agc.scale = __min(8.0f, (m_agc.scale + prefsc) / 2);
				}
			}

			logcat("agc_computer avglevel: %d, currentlevel:%d, overflow: %d, %d, sc: %.2f -> %.2f\n", avglevel, currentlevel, m_agc.overflowcount, m_agc.real_overflowcount, oldsc, m_agc.scale);
			  
			memset(&m_agc.total, 0, sizeof(tLevelStatic)); 
			m_agc.real_overflowcount = 0;
			m_agc.overflowcount = 0;
	   }
	
		void audio_voice_check::clear_cache()
		{
			while (!m_cache.list.empty())
			{
				tFrame* pframe = m_cache.list.front();
				m_cache.list.pop_front();
				freeFrame(pframe);
			}

			m_cache.allen = 0;
			m_cache.endnospeaklen = 0;
			m_cache.spkcounter = 0;
			m_cache.spk_level_len = 0;
			m_cache.staus = tCache::findspk;
			m_cache.bSignal = false;
			if (m_cache.dbg_seqcounter > 65535)
				m_cache.dbg_seqcounter = 0;
		}

		audio_voice_check::tFrame* audio_voice_check::getframe(bool bclear)
		{
			if (!m_enable)
			{
				if (!m_cache.list.empty())
				{
					tFrame* pframe = m_cache.list.front();
					m_cache.list.pop_front(); 
					return pframe;
				}

				return nullptr;
			}

			if (bclear)
			{ 
				if (m_cache.staus == tCache::findspk)
				{
					clear_cache();
					return nullptr;
				}  
				else 
				{
					if (!m_cache.list.empty())
					{
						tFrame* pframe = m_cache.list.front();
						m_cache.list.pop_front(); 
						volume_ctrl(pframe->data(), pframe->len);
                        noise_suppression( pframe->data(), pframe->len, m_cache.list.size() + 1 );
						return pframe; 
					}
					else
					{
						clear_cache();
						return nullptr;
					} 
				} 
			}
			else
			{
				
				if (m_cache.staus == tCache::speaking || m_cache.staus == tCache::spkending)
				{
					tFrame* pframe = nullptr;
					if (m_cache.bSignal)
					{
						tFrame* pframe = m_cache.list.front();
						m_cache.list.pop_front(); 
						m_cache.bSignal = false;
						volume_ctrl(pframe->data(), pframe->len);
                        noise_suppression( pframe->data(), pframe->len, m_cache.list.size() + 1 );
				//		logcat("play frame seq %d, status: %d, vadspk: %d level: %d\n", pframe->dbg_seq, m_cache.staus, pframe->vadspeaking, pframe->esclevel);
						return pframe;
					} 
					else
					{
						return nullptr;
					}
				} 
				else
				{ 
					return nullptr; 
				}
			} 
		}
		 

		void audio_voice_check::freeFrame(tFrame* pframe)
		{ 
			if (m_enable)
			{
				if (pframe->vadspeaking)
					m_cache.spkcounter--;

				if (pframe->esclevel <= m_level_ttl)
					m_cache.spk_level_len -= pframe->len;

			}
				
			m_cache.allen -= pframe->len;

			free(pframe); 
		}


		void audio_voice_check::cache_frame(const void* buf, int len, bool vadspeaking, int esclevel)
		{
            m_cache.dbg_seqcounter++;

            if ( m_enable && tCache::findspk == m_cache.staus )
            {
                // clear idle data
                while ( !m_cache.list.empty() )
                {
                    tFrame* pframe = m_cache.list.front();
                    if ( m_cache.allen < 20 * m_10ms_len && pframe->vadspeaking )
                        break;

                    m_cache.list.pop_front();
                    freeFrame( pframe );
                }

                if ( !vadspeaking && 0 == m_cache.allen)
                    return;
            }


            tFrame* pframe = (tFrame*)malloc( sizeof( tFrame ) + len );
            pframe->len = len;
            pframe->vadspeaking = vadspeaking;
            pframe->esclevel = esclevel;
            pframe->type = m_cache.staus;
            memcpy( pframe->data(), buf, len );

            m_cache.allen += len;

            if ( m_enable )
            {
                if ( vadspeaking )
                    m_cache.spkcounter++;

                if ( vadspeaking && pframe->esclevel <= m_level_ttl )
                {
                    m_cache.spk_level_len += len;
                    if ( m_cache.spk_level_len >= 3 * m_10ms_len )
                    {
                        if ( m_cache.allen > 10 * m_10ms_len )
                        {
                            if ( tCache::findspk == m_cache.staus )
                            {
                                logcat( "speaking start\n" );
                                m_cache.staus = tCache::speaking;
                                m_agc.once.levelcount = 0;
                                m_agc.once.levelsum = 0;
                                m_agc.once.levellen = 0;
                            }
                            else if ( tCache::spkending == m_cache.staus )
                            {
                                m_cache.staus = tCache::speaking;
                            }
                        }
                    }

                    if ( tCache::speaking == m_cache.staus || tCache::spkending == m_cache.staus )
                    {
                        m_agc.once.levelsum += esclevel;
                        m_agc.once.levelcount++;
                        m_agc.once.levellen += len;

                        logcat( "speaking level: %d\n", 10 * m_agc.once.levellen / m_10ms_len );

                        if ( m_agc.once.levellen > 20 * 100 * m_10ms_len )
                            agc_data_syn();
                    }
                }
                else
                {
                    if ( tCache::speaking == m_cache.staus )
                    {
                        m_cache.staus = tCache::spkending;
                        logcat( "speaking spkending\n" );
                    }
                    else if ( tCache::spkending == m_cache.staus )
                    {
                        if ( !vadspeaking || pframe->esclevel > m_level_ttl_nospeak )
                        {
                            m_cache.endnospeaklen += len;

                            if ( m_cache.endnospeaklen > 50 * m_10ms_len )
                            {
                                logcat( "speaking stoped\n" );
                                free( pframe );
                                clear_cache();
                                agc_data_syn();
                                return;
                            }
                        }
                        else
                        {
                            m_cache.endnospeaklen = 0;
                        }
                    }
                }
            }

            pframe->dbg_seq = m_cache.dbg_seqcounter;

            if ( m_cache.list.empty() )
                agc_computer();

            volume_check( pframe->data(), pframe->len );
            m_cache.list.push_back( pframe );
            m_cache.bSignal = true;
		}

		void audio_voice_check::process(const void* buf, int len, int esclevel, bool vadspk)
		{ 
			if (!m_enable)
			{
				cache_frame(buf, len, vadspk, esclevel);
				return;
			}
            update_level( esclevel,vadspk );
			cache_frame(buf, len, vadspk, esclevel); 

			if (m_cache.spk_level_len > 0)
			{
				static int kkk = 0;
				if (kkk++ % 10 == 0)
				{
					logcat("=============== speaking %d, %d, ttl: %d, spklen: %d ms\n", 
						tCache::speaking == m_cache.staus, 
						esclevel <= m_level_ttl && vadspk, m_level_ttl,
						m_cache.spk_level_len * 10 / m_10ms_len);
				}
			} 
		}
		 


		void audio_voice_check::trace_max_level(const void* buf, int len)
		{
			static std::map<int, int> dbg_vals;
			static int kk = 0;
			if (0 != kk)
				return;

			kk = 1;

			dbg_vals.clear();
			int16_t* pSamples = (int16_t*)buf;
			int       samples = len / 2;
			for (int index = 0; index < samples; index++)
			{
				int sample = *(pSamples + index);
				if (sample < 0)
					sample = -sample;

				dbg_vals[sample] = 0;
			}



			std::map<int, int>::reverse_iterator it = dbg_vals.rbegin();
			if (it->first > 200)
			{
				log_out_cat("\n");

				char pbuf[2000];
				char* pfrom = pbuf;


				for (int index = 0; index < 40; index++, it++)
				{
					int dlen = sprintf(pfrom, "%d \t", it->first);

					pfrom += dlen;
				}

				log_out_cat(pbuf);
				log_out_cat("\n");

				kk = 0;
			}

			kk = 0;

		}


		void audio_voice_check::debug_level_analysis(int esclevel)
		{
			static int spkcount = 0;
			static int spkcount2 = 0;
			static tick_t tt = GetTickCount();

			if (esclevel < 40)
				spkcount++;

			if (esclevel < 90)
				logcat("----------------------------------------------------------------- level %d\n", esclevel);

			if (esclevel < 30)
				spkcount2++;

			if (tickcount() - tt > 200)
			{
				logcat("-------------> num: %d %d\n", spkcount, spkcount2);

				tt = tickcount();
				spkcount = 0;
				spkcount2 = 0;
			}

		}

        void audio_voice_check::noise_suppression( void* data, int len, int index )
        {
            if ( !m_enable )
                return;

            if ( m_noise.levels.size() < 30 )
            {
                return;
            }
            if ( index < 2 )
                return;

            int i = 1;
            int level;
            float after_silent = 0;
            float before_silent = 0;
            float after_levelsum = 0;
            float before_levelsum = 0;
            std::list<LevelInfo>::reverse_iterator itCur;
            for ( auto it = m_noise.levels.rbegin(); it != m_noise.levels.rend(); ++it, i++ )
            {
                if ( index > i )
                {
                    if (index - i == 1)
                    {
                        after_levelsum += it->level * 1.15;
                    }
                    else if ( index - i == 2)
                    {
                        after_levelsum += it->level * 1.05;
                    }
                    else
                    after_levelsum += it->level;
                }
                else if ( index < i )
                {
                    if ( i - index  == 1)
                    {
                        before_levelsum += it->level * 1.2;
                    }
                    else if ( i - index == 2 )
                    {
                        before_levelsum += it->level * 1.05;
                    }
                    else
                    {
                        before_levelsum += it->level;
                    }
                }
                else
                {
                    level = it->level;
                    itCur = it;
                }

                if ( index < i + 3 && index > i  )
                {
                    if ( !it->isSpeech )
                    {
                        after_silent++;
                    }
                }
                else if ( index < i  && index > i - 5 )
                {
                    if ( !it->isSpeech )
                    {
                        before_silent++;
                    }
                }
            }
            static int silent = 0;
            before_levelsum = ( before_levelsum / ( m_noise.levels.size() - index ) );
            after_levelsum = ( after_levelsum / ( index - 1 ) );
            float scale = 1;
            m_noise.voice_count++;
            if ( before_levelsum > level + 9 && after_levelsum > level + 6 )
            {
                scale = pow( 10, (float)( level - after_levelsum ) / 20 );
                m_noise.noise_count++;
                itCur->level += (after_levelsum -level )*0.5 ;

            }

            if ( scale < 1 )
            {
                int16_t* p = (int16_t*)data;
                for ( int j = 0; j < len / 2; j++ )
                {
                    p[j] = p[j] * scale;
                }
            }
            m_lpf.Processing( (const int16_t*)data, (int16_t*)data, len / 2 );
            m_noise.voice_quality = (float)m_noise.noise_count / m_noise.voice_count;
            printf( "count=%d,total_count=%d,quality=%f\n", m_noise.noise_count, m_noise.voice_count,m_noise.voice_quality );
        }

        void audio_voice_check::update_level( int level, bool isSpeech )
        {
            if ( m_noise.levels.size() >= 30 )
            {
                m_noise.levels.pop_front();
            }
            LevelInfo li = { level, isSpeech };
            m_noise.levels.push_back( li );
        }

    }
}