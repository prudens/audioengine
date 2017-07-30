#include "server_module.h"
#include "user_manager.h"

int main( int argc, char** argv )
{
    ServerModule::CreateInstance();
    auto usermgr = std::make_shared<UserManager>();
    usermgr->Start();
    system( "pause" );
    usermgr->Stop();
    usermgr.reset();
    ServerModule::DestroyInstance();
    return 0;
}