//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "asio.hpp"
#include <system_error>
#include <fstream>
#include <chrono>
#include "asio/detail/chrono.hpp"
#include <xutility>
#include <regex>
#include "base/time_cvt.hpp"
#include <functional>
#include <atomic>
using asio::ip::tcp;
static std::atomic<int> count = 0;
class session
    : public std::enable_shared_from_this < session >
{
public:
    session( tcp::socket socket )
        : socket_( std::move( socket ) )
    {
        ++count;
        
    }
    ~session()
    {
        //std::cout << --count << std::endl;
    }
    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self( shared_from_this() );
        socket_.async_read_some( asio::buffer( data_, max_length ),
                                 [this, self] ( std::error_code ec, std::size_t length )
        {
            if ( !ec )
            {
                do_write( length );
            }
        } );
    }

    void do_write( std::size_t length )
    {
        auto self( shared_from_this() );
        asio::async_write( socket_, asio::buffer( data_, length ),
                           [this, self] ( std::error_code ec, std::size_t /*length*/ )
        {
            if ( !ec )
            {
                do_read();
            }
        } );
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server( asio::io_context& io_context, short port )
        : acceptor_( io_context, tcp::endpoint( tcp::v4(), port ) ),
        socket_( io_context )
    {
        asio::error_code ec;
        std::cout << "local host ip: "<<acceptor_.local_endpoint( ec ).address().to_string() << std::endl;
        do_accept();
    }

private:
    void do_accept()
    {
        if ( timer_.elapsed() >= 1000 )
        {
            timer_.reset();
            std::cout << count << std::endl;
            count = 0;
        }
        acceptor_.async_accept( socket_,
                                [this] ( std::error_code ec )
        {
            if ( !ec )
            {
                //std::cout << "local host ip: " << acceptor_.local_endpoint( ec ).address().to_string() << std::endl;
                //std::cout <<"recv client ip:" << socket_.remote_endpoint( ec ).address().to_string() << std::endl;
                std::make_shared<session>( std::move( socket_ ) )->start();
            }

            do_accept();
        } ); 
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    Timer timer_;
};

using asio::ip::tcp;

enum { max_length = 1024 };

int test_asio_tcp_client( int argc, char* argv[] )
{
    if (argc <2)
    {
        return 0;
    }
    Timer timer;
    int count = 0;


            asio::io_context io_context;
            tcp::resolver resolver( io_context );
            //asio::connect( s, resolver.resolve( argv[1], "8888" ) );
            tcp::resolver::results_type result = resolver.resolve( argv[1], "8888" );

            tcp::resolver::results_type::const_iterator it = result.begin();
            while ( timer.elapsed() < 1000 )
            {
                try
                {
                asio::error_code ec;
                //std::cout << "before connect remote ip: " << s.remote_endpoint( ec ).address().to_string().c_str();
                tcp::socket s( io_context );
               // tcp::socket::reuse_address reuser( true );

                s.connect( it->endpoint() );
              //  s.set_option( reuser );
                //std::cout << "after connect remote ip: " << s.remote_endpoint( ec ).address().to_string().c_str();
                //std::cout << "Enter message: ";
                char request[max_length];
                strcpy( request, "123" );
                //std::cin.getline( request, max_length );
                size_t request_length = std::strlen( request );
                asio::write( s, asio::buffer( request, request_length ) );
                //std::cout << s.remote_endpoint( ec ).address().to_string().c_str();
                char reply[max_length];
                size_t reply_length = asio::read( s,
                                                  asio::buffer( reply, request_length ) );
                //std::cout << "Reply is: ";
                // std::cout.write( reply, reply_length );
                //std::cout << "\n";
                count++;
            }
                catch ( std::exception& e )
                {
                    std::cerr << "Exception: " << e.what() << "\n";
                }
        }



    std::cout <<"在一秒内运行了：" <<count << std::endl;
    return 0;
}


int test_asio_tcp_server( int argc, char** argv )
{
    try
    {
        asio::io_context io_context;
        server s( io_context, 8888 );
        for ( int i = 0; i < 8;i++ )
        {
            std::async( [&] () { io_context.run(); } );
        }
        io_context.run();
        //io_context.run();
    }
    catch ( std::exception& e )
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

void test_base_ip()
{
    asio::error_code ec;
    using namespace asio::ip;
    //127.0.0.1 是lookback地址
    //224.0.0.0—239.255.255.255是组播地址
    //0.0.0.0 未指定
    for ( const auto&ipstr : {"0.0.0.0","127.0.0.1","192.168.0.1","224.0.0.0","239.255.255.255","240.255.255.255","255.255.255.255"} )
    {
        std::cout << ipstr;


        address_v4 v4;// = address_v4::any();
        v4 = address_v4::from_string( ipstr, ec );
        address addr( v4 );
        std::cout << "\nis loopback = " << addr.is_loopback();
        std::cout << "\nis multicast = " << addr.is_multicast();
        std::cout << "\nis_unspecified=" << addr.is_unspecified();
        std::cout << "\nis ipv4=" << addr.is_v4()<<std::endl;
        tcp::endpoint ep1( addr,8888 );
        tcp::endpoint ep2( make_address(ipstr), 8888 );
        tcp::endpoint ep3( address( address_v4( v4.to_ulong()) ), 8888 );
    }
    std::string strHostname = asio::ip::host_name( ec );
    asio::io_context io_context;
    tcp::socket s(io_context);
    s.open( tcp::v4() );

    s.connect( tcp::endpoint( make_address( "127.0.0.1" ), 8888 ) );
    s.non_blocking( true, ec );

    std::cout << "\nis open: " << s.is_open();
   // s.shutdown( tcp::socket::shutdown_receive, ec );
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }

//    char request[max_length];
    char reply[max_length];


    size_t request_length = 2;
    size_t len = s.write_some( asio::buffer( "123" ), ec );
    if ( ec )
    {
        std::cout << ec.message() << std::endl;
    }
    std::vector<char> str;
    str.resize(4);
    len = s.read_some( asio::buffer( str ), ec );
    if ( ec )
    {
        std::cout << ec.message() << std::endl;
    }
    len = s.read_some( asio::buffer( str ), ec );
    if ( ec )
    {
        std::cout << "[" << ec.value()<<"]"<<ec.message() << std::endl;
       if (ec.value() == asio::error::would_block)
       {
           std::cout << "would_block\n";
       }
    }

    /////send
    s.send( asio::buffer( "123" ),0,ec );
    len = s.available( ec );
    len = s.receive( asio::buffer( reply, len ), 0, ec );
    if ( ec )
    {
        std::cout << ec.message()<<std::endl;
    }
    reply[len] = 0;
    std::cout << reply << std::endl;


    tcp::iostream ios;

  //  std::cout << str;
}

std::string Get_Http_1_1( std::string uri )
{
    std::ostringstream request;
    request << "GET ";
    request << uri;
    request << " HTTP/1.1/n";
    request << "Connection:close /n/n";
    return request.str();
}

// void test_tcp_iostream()
// {
//     using namespace asio::ip;
//     asio::ip::tcp::iostream ios( "www.boost.org", "http" );
//     if (!ios)
//     {
//         return;
//     }
//     asio::ip::tcp::iostream stream;
//     stream.expires_from_now( boost::posix_time::seconds( 60 ) );
//     stream.connect( "pic4.zhimg.com", "http" );
//     stream << "GET /a68158486a927b4d6f39f449bf1eb0ab_l.jpg HTTP/1.0\r\n";
//     stream << "Host: pic4.zhimg.com\r\n";
//     stream << "Accept: */*\r\n";
//     stream << "Connection: close\r\n\r\n";
//     stream.flush();
//     std::ofstream ofs( "D:/lqq.jpg" );
//     stream >> ofs.rdbuf();
// }

#include <asio/ts/internet.hpp>

using asio::ip::tcp;

int test_tcp_iostream( int argc, char* argv[] )
{
    try
    {
        if ( argc != 3 )
        {
            std::cout << "Usage: http_client <server> <path>\n";
            std::cout << "Example:\n";
            std::cout << "  http_client www.boost.org /LICENSE_1_0.txt\n";
            return 1;
        }

        asio::ip::tcp::iostream s;

        // The entire sequence of I/O operations must complete within 60 seconds.
        // If an expiry occurs, the socket is automatically closed and the stream
        // becomes bad.
        asio::ip::tcp::iostream::duration_type dt( std::chrono::seconds( 60 ) );
        s.expires_after( dt );

        // Establish a connection to the server.
        s.connect( argv[1], "http" );
        if ( !s )
        {
            std::cout << "Unable to connect: " << s.error().message() << "\n";
            return 1;
        }

        // Send the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        s << "GET " << argv[2] << " HTTP/1.0\r\n";
        s << "Host: " << argv[1] << "\r\n";
        s << "Accept: */*\r\n";
        s << "Connection: close\r\n\r\n";

        // By default, the stream is tied with itself. This means that the stream
        // automatically flush the buffered output before attempting a read. It is
        // not necessary not explicitly flush the stream at this point.

        // Check that response is OK.
        std::string http_version;
        s >> http_version;
        unsigned int status_code;
        s >> status_code;
        std::string status_message;
        std::getline( s, status_message );
        if ( !s || http_version.substr( 0, 5 ) != "HTTP/" )
        {
            std::cout << "Invalid response\n";
            return 1;
        }
        if ( status_code != 200 )
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            return 1;
        }

        // Process the response headers, which are terminated by a blank line.
        std::string header;
        int len = 0;
        std::regex regex("^Content-Length:\\s*([0-9]+)\\s*");
        std::smatch match;
        while ( std::getline( s, header ) && header != "\r" )
        {
            std::cout << header << "\n";
            if ( std::regex_match( header, match, regex ) )
            {
                if ( match.size() == 2 )
                {
                    std::ssub_match base_sub_match = match[1];
                    std::string base = base_sub_match.str();
                    try{
                        len = stoi( base );
                       }
                       catch ( ... ) {}
                }
            }
        }

        std::cout << "\n";
        if (len>0)
        {
            char*p = new char[len];
            memset( p, 0, len );
            s.read( p, len );
            FILE *file = fopen( "d:/lqq.jpg", "wb+" );
            fwrite( p, 1, len, file );
            fclose( file );
            delete p;
        }
        else
        {
            std::ofstream ofs("zhihu.html");
            ofs << s.rdbuf();
            std::cout << s.rdbuf();
        }

    }
    catch ( std::exception& e )
    {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;

}
void test_asio( int argc, char** argv )
{
    //test_base_ip();
    //test_tcp_iostream( argc, argv );
   // return;
    if ( argc == 2 )
    {
        for ( int i = 0; i < 10;i++ )
        {
            std::async( test_asio_tcp_client, argc, argv );
        }

    }
    else
    test_asio_tcp_server( argc, argv );

    return;
}