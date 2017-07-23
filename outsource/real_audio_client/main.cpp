#include <stdlib.h>
#include <iostream>
#include <memory>
#include "client_module.h"
#include "audio_error.h"
#include "base/async_task.h"
#include "base/timer.h"
#include "user_manager.h"
#include <random>
int main( int argc, char** argv )
{
    ClientModule::CreateInstance();

    auto user_mgr = std::make_shared<UserManager>();
    user_mgr->SetEventCallback( [] ( UID userid, int status )
    {
        if ( status == 4)
        {
            printf( "[%s]µÇÂ¼Ê§°Ü",userid.c_str() );
        }
    });
    user_mgr->ConnectServer();
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis( 1000, 60000 );

    auto userid = std::to_string( dis(gen) );
    user_mgr->Login( userid );
    system("pause");
    user_mgr.reset();
    ClientModule::DestroyInstance();
    
    return 0;
}