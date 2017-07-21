#include "server_module.h"
#include "user_manager.h"

int main( int argc, char** argv )
{
    ServerModule::CreateInstance();
    UserManager* usermgr = new UserManager;
    usermgr->Start();
    system( "pause" );
    usermgr->Stop();
    delete usermgr;
    ServerModule::DestroyInstance();
    return 0;
}