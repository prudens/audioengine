#include <stdlib.h>
#include <iostream>
#include <thread>
#include "engine.h"
using namespace audio_engine;
int main(int argc, char** argv)
{
	std::string filename;
	if (argc == 2)
	{
		filename = argv[1];
	}
	std::thread(run_engine, filename, "d:/").join();
	return 0;
}


