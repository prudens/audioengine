#include <valarray>
#include <iostream>
#include "base/bitarray.h"
#include <assert.h>
#include <bitset>
#include <map>
using namespace audio_engine;
void test_valarray()
{
    using namespace std;
    valarray<float> arr;
    float audio[100];
    for ( int i = 0; i < 100; i++ )
    {
        audio[i] = (float)i;
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
    std::cout<< str<<len << std::endl;

}

struct RTPHeader
{
	int ver;
	int channel;
	int samplerate;
	int seq;
	int crc;
};

void test_bitblock()
{
	RTPHeader header;
	header.ver = 1;
	header.channel = 1;
	header.samplerate = 4;
	header.seq = 1;
	header.crc = 2;
	uint8_t block[4];
	memset(block, 0, 4);
	BitBlock bits(block,4);
	bits.PushBits(header.ver,4);
	bits.PushBits(header.channel,2);
	bits.PushBits(header.samplerate,4);
	bits.PushBits(header.seq,6);
	bits.PushBits(header.crc,16);
	printf("%x,%x,%x,%x",block[0],block[1],block[2],block[3]);
	BitBlock read_bits(block,4);
	RTPHeader readheader;
	readheader.channel = 0;
	readheader.crc = 0;
	readheader.samplerate = 0;
	readheader.seq = 0;
	readheader.ver = 0;
	read_bits.PullBits(readheader.ver,4);
	read_bits.PullBits(readheader.channel, 2);
	read_bits.PullBits(readheader.samplerate, 4);
	read_bits.PullBits(readheader.seq, 6);
	read_bits.PullBits(readheader.crc, 16);
	assert(header.channel == readheader.channel);
	assert(header.samplerate == readheader.samplerate);
	assert(header.ver == readheader.ver);
	assert(header.seq == readheader.seq);
	assert(header.crc == readheader.crc);
}


void test_numeric()
{
	test_bitblock();
}