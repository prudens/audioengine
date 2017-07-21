#include <stdlib.h>
#include <iostream>
#include <memory>
#include "client_module.h"
#include "audio_error.h"
#include "base/async_task.h"
#include "base/timer.h"
#include "user_manager.h"

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
    user_mgr->Login( "123" );
    system("pause");
    user_mgr.reset();
    ClientModule::DestroyInstance();
    
    return 0;
}