#include "webrtc/modules/rtp_rtcp/include/rtp_rtcp.h"
#include "webrtc/transport.h"
#include "base/time_cvt.hpp"
#include <thread>
#include "webrtc/modules/rtp_rtcp/include/rtp_receiver.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_header_parser.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_payload_registry.h"
/*
PT   encoding    media type  clock rate   channels
name( Hz )
___________________________________________________
0    PCMU        A            8, 000       1
1    reserved    A
2    reserved    A
3    GSM         A            8, 000       1
4    G723        A            8, 000       1
5    DVI4        A            8, 000       1
6    DVI4        A           16, 000       1
7    LPC         A            8, 000       1
8    PCMA        A            8, 000       1
9    G722        A            8, 000       1
10   L16         A           44, 100       2
11   L16         A           44, 100       1
12   QCELP       A            8, 000       1
13   CN          A            8, 000       1
14   MPA         A           90, 000       ( see text )
15   G728        A            8, 000       1
16   DVI4        A           11, 025       1
17   DVI4        A           22, 050       1
18   G729        A            8, 000       1
19   reserved    A
20   unassigned  A
21   unassigned  A
22   unassigned  A
23   unassigned  A
dyn  G726 - 40     A            8, 000       1
dyn  G726 - 32     A            8, 000       1
dyn  G726 - 24     A            8, 000       1
dyn  G726 - 16     A            8, 000       1
dyn  G729D       A            8, 000       1
dyn  G729E       A            8, 000       1
dyn  GSM - EFR     A            8, 000       1
dyn  L8          A            var.var.
dyn  RED         A( see text )
dyn  VDVI        A            var.        1

Table 4: Payload types( PT ) for audio encodings

24      unassigned  V
25      CelB        V           90,000
26      JPEG        V           90,000
27      unassigned  V
28      nv          V           90,000
29      unassigned  V
30      unassigned  V
31      H261        V           90,000
32      MPV         V           90,000
33      MP2T        AV          90,000
34      H263        V           90,000
35-71   unassigned  ?
72-76   reserved    N/A         N/A
77-95   unassigned  ?
96-127  dynamic     ?
dyn     H263-1998   V           90,000
Table 5: Payload types (PT) for video and combined encodings
*/

using namespace webrtc;
const CodecInst database[] = {
  { 103, "ISAC", 16000, 480, 1, 32000 },
  { 104, "ISAC", 32000, 960, 1, 56000 },
  { 105, "ISAC", 48000, 1440, 1, 56000 },
  { 107, "L16", 8000, 80, 1, 128000 },
  { 108, "L16", 16000, 160, 1, 256000 },
  { 109, "L16", 32000, 320, 1, 512000 },
  { 111, "L16", 8000, 80, 2, 128000 },
  { 112, "L16", 16000, 160, 2, 256000 },
  { 113, "L16", 32000, 320, 2, 512000 },
  { 0,   "PCMU", 8000, 160, 1, 64000 },
  { 8,   "PCMA", 8000, 160, 1, 64000 },
  { 110, "PCMU", 8000, 160, 2, 64000 },
  { 118, "PCMA", 8000, 160, 2, 64000 },
  { 102, "ILBC", 8000, 240, 1, 13300 },
  { 114, "AMR", 8000, 160, 1, 12200 },
  { 115, "AMR-WB", 16000, 320, 1, 20000 },
  { 116, "CELT", 32000, 640, 1, 64000 },
  { 117, "CELT", 32000, 640, 2, 64000 },
  { 9,   "G722", 16000, 320, 1, 64000 },
  { 119, "G722", 16000, 320, 2, 64000 },
  { 92,  "G7221", 16000, 320, 1, 32000 },
  { 91,  "G7221", 16000, 320, 1, 24000 },
  { 90,  "G7221", 16000, 320, 1, 16000 },
  { 89,  "G7221", 32000, 640, 1, 48000 },
  { 88,  "G7221", 32000, 640, 1, 32000 },
  { 87,  "G7221", 32000, 640, 1, 24000 },
  { 18,  "G729", 8000, 240, 1, 8000 },
  { 86,  "G7291", 16000, 320, 1, 32000 },
  { 3,   "GSM", 8000, 160, 1, 13200 },
  { 120, "opus", 48000, 960, 2, 64000 },
  { 85,  "speex", 8000, 160, 1, 11000 },
  { 84,  "speex", 16000, 320, 1, 22000 },
  { 13,  "CN", 8000, 240, 1, 0 },
  { 98,  "CN", 16000, 480, 1, 0 },
  { 99,  "CN", 32000, 960, 1, 0 },
  { 100, "CN", 48000, 1440, 1, 0 },
  { 106, "telephone-event", 8000, 240, 1, 0 },
  { 127, "red", 8000, 0, 1, 0 },
  { -1,  "Null", -1, -1, -1, -1 }
};

class AudioClient :public Transport, public RtpData
{
public:
    RtpReceiver* receiver=nullptr;
    RtpHeaderParser* parse = nullptr;
    RTPPayloadRegistry *rtp_payload_registry_;
    AudioClient()
    {
        rtp_payload_registry_ = new RTPPayloadRegistry( RTPPayloadStrategy::CreateStrategy( true ) );
        receiver = RtpReceiver::CreateAudioReceiver( Clock::GetRealTimeClock(), nullptr, this, nullptr, rtp_payload_registry_ );
        parse = RtpHeaderParser::Create();

    }
    ~AudioClient()
    {
        delete receiver;
        delete parse;
        delete rtp_payload_registry_;
    }
    virtual bool SendRtp( const uint8_t* packet,
                          size_t length,
                          const PacketOptions& options )
    {
        RTPHeader header;
        if (! parse->Parse( packet, length, &header ) )
        {
            return false;
        }

        PayloadUnion payload_specific;
        if ( !rtp_payload_registry_->GetPayloadSpecifics( header.payloadType,
            &payload_specific ) )
        {
            return false;
        }
        // printf( "send rtp...\n" );
        receiver->IncomingRtpPacket( header, packet + header.headerLength, length - header.headerLength, payload_specific,false);
        return true;
    }
    virtual bool SendRtcp( const uint8_t* packet, size_t length )
    {
        printf( "[%I64u]send rtcp...\n",timestamp() );
        return true;
    }
    virtual int32_t OnReceivedPayloadData( const uint8_t* payloadData,
                                           const size_t payloadSize,
                                           const WebRtcRTPHeader* rtpHeader )
    {
        printf( "[%u:%d]\n",rtpHeader->header.timestamp,rtpHeader->header.sequenceNumber );
        
        return 0;
    }

    virtual bool OnRecoveredPacket( const uint8_t* packet,
                                    size_t packet_length )
    {
        return true;
    }
};

void test_rtp()
{
    AudioClient *client = new AudioClient;
    RtpRtcp::Configuration configure;
    configure.outgoing_transport = client;
    configure.audio = true;
    RtpRtcp* rtp_rtcp = RtpRtcp::CreateRtpRtcp( configure );
    printf("ssrc=%u\n",rtp_rtcp->SSRC());
    rtp_rtcp->SetStartTimestamp(static_cast<int32_t>(timestamp()));
    printf("ts=%u\n",rtp_rtcp->StartTimestamp());
    for ( size_t i = 0; i < sizeof( database ) / sizeof( database[0] )-1; i++ )
    {
        rtp_rtcp->RegisterSendPayload( database[i] );
        bool create_new_payload_type;
        client->rtp_payload_registry_->RegisterReceivePayload( database[i].plname, database[i].pltype, database[i].plfreq, database[i].channels, database[i].rate, &create_new_payload_type );
    }
    rtp_rtcp->SetRTCPStatus( RtcpMode::kCompound );
    uint8_t payload[160] = { 0 };
    strcpy( (char*)payload, "Hello,world" );

    while ( true )
    {
        auto ts = timestamp();
        rtp_rtcp->SendOutgoingData( kAudioFrameSpeech,
                                    database[0].pltype,
                                    ts,
                                    ts,
                                    payload,
                                    160 );
        std::this_thread::sleep_for(milliseconds(10));
        rtp_rtcp->Process();
        RtpPacketLossStats stats ;
        rtp_rtcp->GetRtpPacketLossStats( true, rtp_rtcp->SSRC(), &stats );
        
        //rtp_rtcp->SendRTCP( kRtcpReport );
    }

    delete rtp_rtcp;
}





















void test_rtp_rtcp_test( int argc, char** argv )
{
    test_rtp();
}