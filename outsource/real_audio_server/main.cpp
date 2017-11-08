#include "service.h"

using namespace audio_engine;
int main( int argc, char** argv )
{
	bool running = true;
	while(running)
	{
		Service service;
		printf( "Æô¶¯·şÎñ\n" );
		service.Run();
		std::string cmd;
		while(true)
		{
			std::cin >> cmd;
			if(cmd == "q")
			{
				running = false;
				break;
			}
			else if( cmd == "restart" )
			{
				break;
			}
		}
	}


    return 0;
}