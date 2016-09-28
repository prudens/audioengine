#include "bitarray.h"
#include <stdexcept>
#include <cassert>
uint8_t op_and[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
bitarray::bitarray( size_t N )
{
    assert( N % 8 == 0 ) ;
    m_bytes = ( N + 7 ) / 8;
    m_array = new uint8_t[m_bytes ];
    memset( m_array, 0, m_bytes );
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
    if (pos >= m_nBits)
    {
        throw std::out_of_range("pos is out of range");
    }
    for ( int i = 0; i< nbit;i++ )
    {
        set( pos+i, (value&( 1 << i )) != 0 );
    }
}

void bitarray::set( size_t pos, bool value /*= true */ )
{
    if ( pos >= m_nBits )
    {
        throw std::out_of_range( "pos is out of range" );
    }
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

bool bitarray::set( int start, std::vector<std::pair< int, int>>slice_list )
{
    size_t max = start;
    for (const auto& v:slice_list)
    {
        if (v.second > 32)
        {
            return false;
        }
        max += v.second;
        if (max >= m_nBits)
        {
            return false;
        }
    }
    size_t pos = start;
    for ( const auto& v : slice_list )
    {
        for ( int i = v.second - 1; i >= 0; i--, pos++ )
        {
            size_t cur_byte = pos / 8;
            size_t idx = 7 - pos % 8;
            if ( ( v.first >> i ) & 1 )
            {
                m_array[cur_byte] |= 1 << idx;
            }
            else
            {
                m_array[cur_byte] &= op_and[idx];
            }
        }
    }
    return true;
}

bool bitarray::test( size_t pos )const
{
    if ( pos >= m_nBits )
    {
        throw std::out_of_range( "pos is out of range" );
    }
    size_t cur_byte = pos / 8;
    size_t idx = 7- pos % 8;
    return 0 != (m_array[cur_byte] & 1<<idx);
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
    if ( pos >= m_nBits )
    {
        throw std::out_of_range( "pos is out of range" );
    }
    size_t cur_byte = pos  / 8;
    size_t idx = pos % 8;
    m_array[cur_byte] ^= (uint8_t)1 << idx;
    return *this;
}

std::string bitarray::to_string()
{
    std::string str;
    str.reserve( m_bytes );
    for ( size_t i = 0; i < m_nBits;i++ )
    {
        str.push_back(test( i ) ? '1' : '0');
    }
    return str;
}

std::vector<uint32_t> bitarray::to_uint32s()
{
    std::vector<uint32_t> vec;
    size_t i = 0;
    for (; i < m_bytes-3; i+=4 )
    {
        uint32_t v = 0;
        v += m_array[i]<<24;
        v += (uint32_t)m_array[i + 1] << 16;
        v += (uint32_t)m_array[i + 2] << 8;
        v += (uint32_t)m_array[i + 3];
        vec.push_back( v );
    }
    uint32_t v = 0;
    if (i<m_bytes)
    {
        v += m_array[i]<<24;
    }
    else
    {
        return vec;
    }
    if (++i<m_bytes)
    {
        v += (uint32_t)m_array[i + 1] << 16;
    }
    if ( ++i < m_bytes )
    {
        v += (uint32_t)m_array[i + 1] << 8;
    }
    
    vec.push_back( v );
    return vec;
}

uint32_t bitarray::to_uint32()
{
    size_t i = 0;
    uint32_t v = 0;
    if ( i < m_bytes )
    {
        v += m_array[i++]<<24;
    }
    if ( i < m_bytes )
    {
        v += (uint32_t)m_array[i++] << 16;
    }
    if ( i < m_bytes )
    {
        v += (uint32_t)m_array[i++] << 8;
    }
    if (i < m_bytes)
    {
        v += (uint32_t)m_array[i];
    }
    return v;
}

uint8_t* bitarray::data()
{
    return m_array;
}

