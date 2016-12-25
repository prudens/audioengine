#include "atp.h"
#include "rtp_impl.h"

RTPImpl::RTPImpl()
{
    clock_ = Clock::GetRealTimeClock();
    receiver_stat_ = ReceiveStatistics::Create( clock_ );
    rtp_payload_registry_ = new RTPPayloadRegistry( RTPPayloadStrategy::CreateStrategy(true) );
    rtp_parse_ = RtpHeaderParser::Create();
    rtp_receiver_ = RtpReceiver::CreateAudioReceiver( clock_, nullptr, this, nullptr, rtp_payload_registry_ );
    RtpRtcp::Configuration configure;
    configure.audio = true;
    configure.bandwidth_callback = this;
    configure.clock = clock_;
    configure.outgoing_transport = this;
    configure.rtt_stats = this;
    configure.send_bitrate_observer = this;
    configure.receive_statistics = receiver_stat_;
    rtp_rtcp_ = RtpRtcp::CreateRtpRtcp(configure);
}

RTPImpl::~RTPImpl()
{
    delete receiver_stat_;
    delete rtp_payload_registry_;
    delete rtp_parse_;
    delete rtp_receiver_;
    delete rtp_rtcp_;
    delete clock_;
}

bool RTPImpl::EncodePacket( void* payload, size_t len_of_byte )
{
    return true;
}

bool RTPImpl::DecodePacket( void* packet, size_t len_of_byte )
{
    return true;
}

bool RTPImpl::SendRtp( const uint8_t* packet, size_t length, const webrtc::PacketOptions& options )
{
    return true;
}

bool RTPImpl::SendRtcp( const uint8_t* packet, size_t length )
{
    return true;
}

int32_t RTPImpl::OnReceivedPayloadData( const uint8_t* payloadData,
                                        const size_t payloadSize,
                                        const webrtc::WebRtcRTPHeader* rtpHeader )
{
    return true;
}

bool RTPImpl::OnRecoveredPacket( const uint8_t* packet, size_t packet_length )
{
    return true;
}

void RTPImpl::OnReceivedEstimatedBitrate( uint32_t bitrate )
{
}

void RTPImpl::OnReceivedRtcpReceiverReport( const webrtc::ReportBlockList& report_blocks,
                                            int64_t rtt,
                                            int64_t now_ms )
{
}

void RTPImpl::OnRttUpdate( int64_t rtt )
{

}

int64_t RTPImpl::LastProcessedRtt() const
{
    return 0;
}

void RTPImpl::Notify( const webrtc::BitrateStatistics& total_stats,
                      const webrtc::BitrateStatistics& retransmit_stats,
                      uint32_t ssrc )
{

}
