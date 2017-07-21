#ifdef _DEBUG
#define CONFIGURE "Debug/"
#else
#define CONFIGURE "Release/"
#endif
#ifdef WIN64
#define PLATFORM "x64/"
#else 
#define PLATFORM ""
#endif 

#define PATH_LIB   "../../build/winx/"""PLATFORM""CONFIGURE


#pragma comment(lib,PATH_LIB"audio_base.lib")

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dsound.lib")
