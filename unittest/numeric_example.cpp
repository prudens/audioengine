#include <valarray>
#include <iostream>
#include "base/bitarray.h"
#include <assert.h>
#include <bitset>
void test_valarray()
{
    using namespace std;
    valarray<float> arr;
    float audio[100];
    for ( int i = 0; i < 100; i++ )
    {
        audio[i] = i;
    }
    arr = valarray<float>( audio, 100 );
    arr += 1;
    auto newarr = arr.cshift( 10 );
    for ( auto v : newarr )
    {
        std::cout << v << " ";
    }
    std::cout << arr.sum() << endl;
    std::cout << arr.min() << endl;
    std::cout << arr.max() << endl;
    arr = newarr[slice{ 0, 10, 2 }];
    newarr = arr.apply( [] ( float v ) { return v*v; } );
}

void test_bitarray()
{
    bitarray arr( 128 );
//     uint32_t v=3;
//     arr.set(0, v, 2 );
//     arr.set( 2 );
//     arr[3] = true;
//     arr[1] = false;
//     assert( arr.test( 0 ) == true );
//     assert( arr.test( 1 ) == false );
//     assert( arr[2] == true );
//     std::string str = arr.to_string();
//     v = 137;
//     arr.set( 0, v,8 );
//     v = arr.touint32();
//     std::cout << v<<"\n";
//     std::cout << arr.to_string();
    int *test = new int(0x12345678);
    std::pair<int, int> p = { 1, 2 };
    std::vector<std::pair<int, int>>v1 = {
        { 0, 2 }, { 3, 3 }, { 4, 3 }, {*test,32}
    };
    arr.set(0, v1 );
    *test = arr.to_uint32();
    std::cout << arr.to_string() << "to uint32=0x" <<std::hex<< *test << std::endl;

    std::vector<std::pair<int, int>>v2 = {
        { 0, 2 }, { 0, 3 }, { 0, 3 }, { *test, 32 }
    };
    arr.get( 0, v2 );
    std::cout << "get " << v2[0].first << " " << v2[1].first << " " << v2[2].first <<" "<<std::hex<<"0x"<<v2[3].first<< std::endl;
    
    for ( int i = 0; i < 32; i++ )
    {
        std::bitset<32> bs;
        bs.flip();
        bs.set( i, 0 );
        std::cout <<"0x"<< bs.to_ulong() <<",";
    }

    bitarray arr2(32);
    arr2.flip();
    arr2.set( 0, 0 );
    uint32_t u = arr2.to_uint32l();
    std::cout << std::hex << u<<std::endl;
}

void test_array()
{
  //  test_bitarray();

    char* p = new char[10];
    memset( p, '\0', 10 );
    p[2] = 'a';
    std::string str(p,10);
    const char* pp = str.c_str();
    size_t len = str.length();
    len = str.size();
    std::cout  << str<<len << std::endl;
}