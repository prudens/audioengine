#include "silent_voice_filter.h"
#include <assert.h>
namespace audio_engine{
#define MAX_OVERFLOW_CNT 4
	SilentVoiceFilter::SilentVoiceFilter ()
	{

	}

	SilentVoiceFilter::~SilentVoiceFilter ()
	{

	}

	void SilentVoiceFilter::Enable ( bool enable )
	{
		_enbale = enable;
	}

	bool SilentVoiceFilter::IsEnable () const
	{
		return _enbale;
	}

	void SilentVoiceFilter::SetRMSThreshold ( int level )
	{
		assert ( level < 127 && level >= 0 );
		_rms_level_threshold = level;
	}

	int SilentVoiceFilter::GetRMSThreshold () const
	{
		return _rms_level_threshold;
	}

	void SilentVoiceFilter::SetSilentThreshold( int count )
	{
		_silent_cnt_threshold = count;
	}

	int SilentVoiceFilter::GetSilentThreshold() const
	{
		return _silent_cnt_threshold;
	}

	AudioBufferPtr SilentVoiceFilter::Process( AudioBufferPtr buf )
	{
		if( !_enbale )
		{
			return buf;
		}

		if( _silent_cnt < _silent_cnt_threshold )
		{
			_buf_list.push_back ( buf );
		}

		if( buf->silent || buf->rms >= _rms_level_threshold )
		{
			_silent_cnt++;
			_rms_level_overflow_cnt = 0;
			_buf_list_cache.clear ();
		}
		else
		{
			if( _silent_cnt >= _silent_cnt_threshold - 4 )
			{
				_silent_cnt++;
				_rms_level_overflow_cnt++;
				_buf_list_cache.push_back ( buf );
			}
			if( _rms_level_overflow_cnt > MAX_OVERFLOW_CNT )
			{
				_silent_cnt = 0;
				_buf_list = std::move ( _buf_list_cache );
				_buf_list_cache.clear ();
				_rms_level_overflow_cnt = 0;
			}
		}

		if( _buf_list.empty () )
		{
			auto p = _buf_list.front ();
			_buf_list.pop_front ();
			return p;
		}
		return nullptr;
	}

	void SilentVoiceFilter::Reset ()
	{
		_rms_level_overflow_cnt = 0;
		_silent_cnt = 0;
		_buf_list.clear ();
		_buf_list_cache.clear ();
	}
}