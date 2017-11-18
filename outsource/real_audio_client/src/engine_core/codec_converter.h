#pragma once
#define _OPUS_FRAME_SPAN  40

enum
{
    CODEC_AAC,
    CODEC_OPUS,
};

#define AACObjectMain         1
#define AACObjectLC           2
#define AACObjectSSR          3
#define AACObjectLTP          4
#define AACObjectHE           5
#define AACObjectScalable     6
#define AACObjectERLC         17
#define AACObjectLD           23
#define AACObjectHE_PS        29
#define AACObjectELD          39
#include <cstdint>
#include <cstddef>

class CodecConverter
{
public:
    virtual ~CodecConverter(){}
	virtual int Initialize(){ return 0; }
    virtual int Process( const void* inputData, std::size_t inputSize, void* outputData, size_t& outputSize ) { return -1; }
    virtual void Destroy() { delete this; }
};

class CodecFactory
{
public:
	CodecFactory();
	CodecConverter* Create(int src_type, int dst_type, int samplerate, int channel);
    void SetHardwareCodec( bool open );
    bool IsHardwareCodec() const;
private:
	bool _hardware_codec = false;
};

