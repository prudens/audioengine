//
//  CircularBuffer.h
//  
//
//  Created by Philadelphia Game Lab on 6/30/14.
//
//

#ifndef _CircularBuffer_h
#define _CircularBuffer_h

#include <cstdint>

template <class T>
class CircularBuffer{
protected:
    /*Declaring variables for use by circular buffer functions
     _begIndex is buffer head, 3DMixer will read from here
     _endIndex is buffer tail, buffer will write from wav file here
     _capacity is circular buffer capacity
     */
    size_t _begIndex, _endIndex, _capacity;
    
    //_size is the current size of the buffer, amount of data written to buffer
   // std::atomic<size_t> _size;
    size_t _size;
    //pointer to circular buffer, start writing data from here
    T *_data;
    
public:
       CircularBuffer(size_t capacity)
    :   _begIndex(0)
    ,   _endIndex(0)
    ,   _size(0)
    ,   _capacity(capacity)
    {
        _data = new T[capacity*sizeof(T)];
    }
    
    ~CircularBuffer()
    {
        delete[] _data;
    }
    
    //Define write audio data to circular buffer
    size_t write(T *dataPtr, size_t samples);
    size_t writeSizeRemaining()const;
    size_t readSizeRemaining()const;
    template <class V>
    size_t read(V *dataPtr, size_t samples);
    
    void clear();
};

template <class T>
size_t CircularBuffer<T>::write(T *dataPtr, size_t samples)
{
    size_t capacity = _capacity;
    size_t samples_to_write = std::min(samples, capacity-_size);
    
    _size += samples_to_write;
    //writing to buffer
    
    if (samples_to_write <= capacity - _endIndex) {
        // samples to write will fit in buffer without wrapping
        std::copy( &dataPtr[0], &dataPtr[samples_to_write], &_data[_endIndex] ); // 提高效率
        _endIndex += samples_to_write;
        if (_endIndex == capacity) {
            _endIndex = 0;
        }
    } else {
        // TODO: pretty sure this can be done with modulo arithmetic...
        size_t size1 = capacity - _endIndex;

        std::copy( &dataPtr[0], &dataPtr[size1], &_data[_endIndex] ); // 提高效率
        size_t size2 = samples_to_write - size1;

        std::copy( &dataPtr[size1], &dataPtr[size2+size1], &_data[0] ); // 提高效率
        _endIndex = size2;
    }
    
    return samples_to_write;
}

template <class T>
size_t CircularBuffer<T>::writeSizeRemaining()const
{
    return (_capacity - _size);
}

template <class T>
size_t CircularBuffer<T>::readSizeRemaining()const
{
    return _size;
}

template <class T> template <class V>
size_t CircularBuffer<T>::read(V *dataPtr, size_t samples)
{
    if (samples == 0){
        return 0;
    }
    size_t capacity = _capacity;
    size_t samples_to_read;
    if (samples >= _size){
        samples_to_read = _size;
    }else{
        samples_to_read = samples;
    }
    // Read in a single step
    if (samples_to_read <= capacity - _begIndex){
        std::copy( &_data[_begIndex], &_data[_begIndex + samples_to_read], dataPtr );
        _begIndex += samples_to_read;
        if (_begIndex == capacity){
            _begIndex = 0;
        }
    }else{
        size_t size_1 = capacity - _begIndex;

        std::copy(&_data[_begIndex],&_data[_begIndex+size_1],dataPtr);
        size_t size_2 = samples_to_read - size_1;
        std::copy( &_data[0], &_data[size_2], &dataPtr[size_1] );
        _begIndex = size_2;
    }
    _size -= samples_to_read;
    return samples_to_read;
}

template <class T>
void CircularBuffer<T>::clear() {
    _size = 0;
    _begIndex = 0;
    _endIndex = 0;
}

using CircularAudioBuffer = CircularBuffer<int16_t>;
#endif
