#ifdef _DEBUG
#define PATH_LIB   "../../build/winx/Debug/"
#else
#define PATH_LIB "../../build/winx/Release/"
#endif

#pragma comment(lib,PATH_LIB"audio_device.lib")
#pragma comment(lib,PATH_LIB"audio_effect.lib")
#pragma comment(lib,PATH_LIB"audio_io.lib")
#pragma comment(lib,PATH_LIB"audio_base.lib")
#pragma comment(lib,PATH_LIB"audio_processing.lib")
#pragma comment(lib,PATH_LIB"libmpg123.lib")
#pragma comment(lib,PATH_LIB"libmp3lame.lib")
#pragma comment(lib,PATH_LIB"aac.lib")
#pragma comment(lib,PATH_LIB"g7221.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
#pragma comment(lib,"winmm.lib")