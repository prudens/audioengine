//#define _TEST
#ifdef _TEST
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "asio.hpp"
#include "base/time_cvt.hpp"
using asio::ip::udp;

enum { max_length = 1024 };

int main( int argc, char* argv[] )
{
    try
    {

        asio::io_context io_context;

        udp::socket s( io_context, udp::endpoint( udp::v4(), 0 ) );

        int64_t t[3];
        t[0] = timestamp();
        size_t request_length = sizeof( t );
        s.send_to( asio::buffer( (char*)t, request_length ), udp::endpoint( asio::ip::make_address_v4("127.0.0.1"), 8080 ) );

        udp::endpoint sender_endpoint;
        size_t reply_length = s.receive_from(
            asio::buffer( (char*)t, request_length ), sender_endpoint );
        t[2] = timestamp();
        std::cout << "t1:" << t[0] << " t2:" << t[1] << "  t3:" << t[2];
        std::cout << "Ïà²îÁË£º" << ( t[0] + t[2] ) / 2 - t[1] << "ms";
    }
    catch ( std::exception& e )
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
#else
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
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis( 1000, 60000 );

    auto userid = std::to_string( dis(gen) );
    user_mgr->Login( userid );
    system("pause");
    user_mgr->Logout();
    system( "pause" );
    user_mgr.reset();
    ClientModule::DestroyInstance();
    
    return 0;
}
#endif