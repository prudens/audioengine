#pragma once
#include <list>
#include <memory>

namespace snail{
namespace audio{
#define CACHE_FRAME_NUM 4	  
	class AudioVoiceCheck
	{
	public:
		struct tFrame
		{
			tFrame(const void* data, int len)
            {
                buffer = new char[len];
                memcpy( buffer, data, len );
                this->len = len;
            }
			~tFrame()
            {
                delete buffer;
            } 
			int   len;  
		    char* buffer;
		    char* data(){ return buffer; }
		};

		typedef std::shared_ptr<tFrame> frameptr;    
	public:
		AudioVoiceCheck();
		~AudioVoiceCheck();
		void Reset( int frame_time);
        void Enable( bool bEnable );
        bool IsEnable();
        void SetLevel( int level );

		void Process( const void* buf, int len, bool vadspk, int esclevel);
		frameptr GetFrame();

        void UpdateSpeakerNumber( int spknum );

	private:  
        bool           m_enable = true;
		int            m_mute_cnt = 0; 
		int            m_sent_mute_ttl = 200;
		int            m_mute_level_ttl = 60;
        int            m_level_overflow_cnt = 0;
        int            m_mute_level_gain = 0;
        std::list<frameptr> m_send_frames;
        std::list<frameptr> m_cache_frames;
	};
}}
 