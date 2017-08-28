#pragma once
#include <cstdint>
#include <string>
#include <vector>
// 默认网络序，字节内部是从又到左，但是切分block的时候，不再是以字节为单位。
class bitarray
{
public:
    class reference
    {	// proxy for an element
        friend class bitarray;

    public:
        ~reference()
        {	// destroy the object
        }

        reference& operator=( bool _Val )
        {	// assign Boolean to element
            _Pbitset->set( _Mypos, _Val );
            return ( *this );
        }

            reference& operator=( const reference& _Bitref )
        {	// assign reference to element
            _Pbitset->set( _Mypos, bool( _Bitref ) );
            return ( *this );
        }

            reference& flip()
        {	// complement stored element
            _Pbitset->flip( _Mypos );
            return ( *this );
        }

            bool operator~( ) const
        {	// return complemented element
            return ( !_Pbitset->test( _Mypos ) );
        }

            operator bool() const
        {	// return element
            return ( _Pbitset->test( _Mypos ) );
        }

    private:
        reference()
            : _Pbitset( 0 ), _Mypos( 0 )
        {	// default construct
        }

        reference( bitarray& _Bitset, size_t _Pos )
            : _Pbitset( &_Bitset ), _Mypos( _Pos )
        {	// construct from bitset reference and position
        }

        bitarray *_Pbitset;	// pointer to the bitset
        size_t _Mypos;	// position of element in bitset
    };

    bitarray( size_t N );
    ~bitarray();
    size_t bits()const;
    void set( size_t pos, uint32_t value, int nbit);
    void set( size_t pos, bool value = true );
    bool set(int pos, std::vector<std::pair< int, int>> v );//start 是位数的基数。第一个是int 是value,第二个是去最低的有效位数。从0开始累加
    bool get( int pos, std::vector<std::pair<int, int>>&blocks );
    bool test( size_t pos )const;
    reference operator []( size_t pos );
    bool operator[]( size_t _Pos )const;
    bitarray &flip();
    bitarray&flip(size_t pos);
    std::string to_string();
    std::vector<uint32_t> to_uint32s();
    uint32_t              to_uint32(); // 大端，网络序
    uint32_t              to_uint32l();//小端
    uint8_t*data();

private:
    uint8_t* m_array;
    size_t   m_nBits;
    size_t   m_bytes;
};

class BitBlock
{
public:
	BitBlock(uint8_t* block, size_t length)
	{
		_block = block;
		memset(_block,0,length);
		_length = length;
	}
	template<typename T, typename = std::enable_if<std::is_integral<T>::value,T>::type>
	void PushBits(T v, int nbit)
	{
		for (int i = 0; i < nbit; i++)
		{
			int b = 1 & ( v >> ( nbit - i - 1 ) );
			_block[bit_idx / 8] |= b << (7-bit_idx % 8);
			bit_idx++;
		}
	}
private:
	uint8_t* _block;
	size_t _length;
	int bit_idx = 0;
};