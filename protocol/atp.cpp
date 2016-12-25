#pragma once
#include "atp.h"
#include "rtp_impl.h"
AudioTransportProtocol* AudioTransportProtocol::Create()
{
    return new RTPImpl();
}