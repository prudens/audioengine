#pragma once
#include <memory>

class ATPDataReceiver
{
public:
    virtual bool  ReceiverAudioPacket( void* packet, size_t len_of_byte ) = 0;
    virtual bool  ReceiverPayloadData( void* payload, size_t len_of_byte ) = 0;
    virtual bool  ReceiverCtrlPacket( void* packet, size_t len_of_byte ) = 0;
    virtual bool  ReceiverCtrlInformation( void* ctrl_info, size_t len_of_byte ) = 0;
};

class AudioTransportProtocol
{
public:
    AudioTransportProtocol*Create();
    virtual bool EncodePacket( void* payload, size_t len_of_byte ) = 0;
    virtual bool DecodePacket( void* packet, size_t len_of_byte ) = 0;
};



typedef AudioTransportProtocol* PATP;
typedef std::shared_ptr<AudioTransportProtocol> ATPPtr;