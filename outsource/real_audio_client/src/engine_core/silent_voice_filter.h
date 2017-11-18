#pragma once
#include <list>
#include "audio_buffer.h"
namespace audio_engine{
	class SilentVoiceFilter
	{
	public:
		SilentVoiceFilter ();
		~SilentVoiceFilter ();
	public:
		void Enable( bool enable );
		bool IsEnable()const;
		void SetRMSThreshold( int level );
		int  GetRMSThreshold()const;
		void SetSilentThreshold(int count);
		int  GetSilentThreshold()const;
		AudioBufferPtr Process( AudioBufferPtr buf );
		void Reset ();
	private:
		bool _enbale = false; //是否开启功能
		int _rms_level_threshold = 127;
		int _silent_cnt_threshold = 0;
		int _rms_level_overflow_cnt = 0;
		int _silent_cnt = 0;
		std::list<AudioBufferPtr> _buf_list;
		std::list<AudioBufferPtr> _buf_list_cache;
	};
}