#include <fstream>  
#include <algorithm>  
#include <string>  
#include <sstream>  
#include <vector>
#include <functional>
void ReadLine( const char *pfilename, std::function<bool(std::string&)>cb )
{
    std::locale::global( std::locale( "" ) );
    std::ifstream ifs( pfilename );
    std::string str;
    while ( getline( ifs, str ) )
    {
        if(!cb( str ))
            break;
    }
    ifs.close();
    setlocale( LC_ALL, "C" );
}