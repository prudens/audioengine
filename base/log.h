#pragma once
#include"time_cvt.hpp"

enum LogLevel
{
	LEVEL_CLOSE = 0,
	LEVEL_VERBOSE = 1,
	LEVEL_DEBUG = 2,
	LEVEL_INFO = 3,
	LEVEL_WARN = 4,
	LEVEL_ERROR = 5,
};


template <typename ... Args> void logcat(const char * format, Args const & ... args)
{
#if defined(_WINDOWS)||defined(_WIN32)
	if (sizeof...(args) == 0)
	{
		OutputDebugStringA(FormatTime().c_str());
		OutputDebugStringA("[SnailAudioEngine]");
		OutputDebugStringA(format);
	}
	else
	{
		OutputDebugStringA(FormatTime().c_str());
		OutputDebugStringA("[SnailAudioEngine]");
		char fmt_buf[1024];
		sprintf(fmt_buf, format, args ...);
		OutputDebugStringA(fmt_buf);
	}
#elif _ANDROID
	if (sizeof...(args) == 0)
	{
		__android_log_print(ANDROID_LOG_DEBUG, "SnailAudioEngine", "%s", format);
	}
	else
	{
		__android_log_print(ANDROID_LOG_DEBUG, "SnailAudioEngine", format, args ...);
	}
#else
	if (sizeof...(args) == 0)
	{
		printf("%s", FormatTime().c_str());
		printf("[SnailAudioEngine]%s", format);
	}
	else
	{
		printf("%s", FormatTime().c_str());
		printf("[SnailAudioEngine]");
		printf(format, args ...);
	}
#endif
}

class Logger
{
public:
	void setLevel(int level)
	{
		if (level == LEVEL_CLOSE)
		{
			level = LEVEL_ERROR + 1;
		}
		_level = level;
	}
	template <typename ... Args>
	void v(const char* fmt, Args const & ... args)
	{
		if (_level <= LEVEL_VERBOSE)
		{
			logcat(fmt, args...);
		}
	}

	template <typename ... Args>
	void d(const char* fmt, Args const & ... args)
	{
		if (_level <= LEVEL_DEBUG)
		{
			logcat(fmt, args...);
		}
	}
	template <typename ... Args>
	void i(const char* fmt, Args const & ... args)
	{
		if (_level <= LEVEL_INFO)
		{
			logcat(fmt, args...);
		}
	}
	template <typename ... Args>
	void w(const char* fmt, Args const & ... args)
	{
		if (_level <= LEVEL_WARN)
		{
			logcat(fmt, args...);
		}
	}
	template <typename ... Args>
	void e(const char* fmt, Args const & ... args)
	{
		if (_level <= LEVEL_ERROR)
		{
			logcat(fmt, args...);
		}
	}
private:
	int _level = LEVEL_ERROR + 1;
};