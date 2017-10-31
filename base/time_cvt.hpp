#pragma once
#ifndef TIME_CVT_H
#define TIME_CVT_H
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <xlocale>

#if defined(WIN32) || defined(_WIN32)
#if _MSC_VER < 1800
#error "should use VS2013 or above version implementation"
#endif
#else
#if __cplusplus < 201103L
#error "should use C++11 implementation"
#endif
#endif//

namespace audio_engine
{
	inline  uint64_t TimeStampMs()
	{
		using namespace std::chrono;
		/*system_clock::now() is current time by second,
		  and default construct is start GMT time */
		return duration_cast<milliseconds>(
			system_clock::now() - time_point<system_clock>() ).count();

		// return time( nullptr );
	}

	inline std::string FormatTime( uint64_t ts )
	{
		std::time_t t = ts;
		time_t tt = (time_t)( t / 1000 );
		int    micsec = t % 1000;
		struct tm *local_time = NULL;
		local_time = localtime( &tt );
		char szBuffer[32] = { 0 };
		int len = strftime( szBuffer, sizeof( szBuffer ), "%Y-%m-%d %H:%M:%S", local_time );
		sprintf( szBuffer + len, ".%d", micsec );

		return std::string( szBuffer );
	}

	inline std::string FormatTime()
	{
		return FormatTime( TimeStampMs() );
	}

	// 计时器
	class ChronoMeter
	{
	public:
		ChronoMeter() : m_begin( std::chrono::high_resolution_clock::now() ) {}
		void reset() { m_begin = std::chrono::high_resolution_clock::now(); }

		//默认输出毫秒
		int64_t Elapsed() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

		//微秒
		int64_t ElapsedMicro() const
		{
			return std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

		//纳秒
		int64_t ElapsedNano() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

		//秒
		int64_t ElapsedSeconds() const
		{
			return std::chrono::duration_cast<std::chrono::seconds>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

		//分
		int64_t ElapsedMinutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

		//时
		int64_t ElapsedHours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>( std::chrono::high_resolution_clock::now() - m_begin ).count();
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
	};

}
#endif