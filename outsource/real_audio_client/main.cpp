#include "client_module.h"
#include "socket_manager.h"
int main( int argc, char** argv )
{
    ClientModule::CreateInstance();
    auto sm = ClientModule::GetInstance()->GetSocketManager();
    socket_t id = sm->Connect( "127.0.0.1", 8888 );
    char buf[1024] = { 0 };
    strcpy( buf, "Hello,World" );
    size_t length = strlen( buf ) + 1;
    if ( id != 0 )
    {
        printf( "connect server 127.0.0.1 success!!!\n" );

        sm->Write( id, buf,length);
        length = 1024;
        length = sm->Read( id, buf, length);
        printf("read:%s\n",buf);
        sm->AsyncWrite( id, buf, length, [&] (std::error_code ec, size_t len) 
        {
            printf( "write:%d",len );
            len = 1024;
            sm->AsyncRead( id, (void*)buf, len, [&] ( std::error_code ec, size_t len ) {
            
                printf( "async read:%s\n", buf );
            } );
        } );
    }
    else
    {
        printf( "connect server 127.0.0.1 failed!!!\n" );
    }

    system("pause");
    ClientModule::DestroyInstance();
    
    return 0;
}