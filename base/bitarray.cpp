#include "bitarray.h"
uint8_t op_and[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
bitarray::bitarray( size_t N )
{
    m_bytes = ( N + 7 ) / 8;
    m_array = new uint8_t[m_bytes ];
    m_nBits = N;
}

bitarray::~bitarray()
{
    delete m_array;
}

size_t bitarray::bits() const
{
    return m_nBits;
}

void bitarray::set( size_t pos, uint32_t value, int nbit )
{
    for ( int i = 0; i< nbit;i++ )
    {
        set( pos+i, (value&( 1 << i )) != 0 );
    }
}

void bitarray::set( size_t pos, bool value /*= true */ )
{
    size_t cur_byte = pos / 8;
    size_t idx = pos % 8;
    if (value)
    {
        m_array[cur_byte] |= 1 << idx;
    }
    else
    {
        m_array[cur_byte] &= op_and[idx];
    }
}

bool bitarray::test( size_t pos )const
{
    size_t cur_byte = pos / 8;
    size_t idx = pos % 8;
    return 0 != (m_array[cur_byte] & op_and[idx]);
}

bitarray::reference bitarray::operator[]( size_t pos )
{
    return bitarray::reference( *this, pos );
}
bool bitarray::operator[]( size_t _Pos )const
{
    return test( _Pos );
}

bitarray & bitarray::flip()
{
    for ( size_t i = 0; i < m_bytes;i++ )
    {
        m_array[i] ^= m_array[i];
    }
    return *this;
}

bitarray& bitarray::flip( size_t pos )
{
    size_t cur_byte = pos  / 8;
    size_t idx = pos % 8;
    m_array[cur_byte] ^= (uint8_t)1 << idx;
    return *this;
}

std::string bitarray::to_string()
{
    return std::string();
}

std::vector<uint32_t> bitarray::touint32()
{
    return std::vector<uint32_t>();
}

