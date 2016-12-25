#include "atp.h"
#include "atp_typedef.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_rtcp.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_receiver.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_header_parser.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_payload_registry.h"
#include "webrtc/transport.h"

using namespace webrtc;
class RTPImpl 
    : public AudioTransportProtocol
    , public webrtc::Transport
    , public webrtc::RtpData
    , public webrtc::RtcpBandwidthObserver
    , public webrtc::RtcpRttStats
    , public webrtc::BitrateStatisticsObserver
{
public:
    RTPImpl();
    ~RTPImpl();
public://AudioTransportProtocol
    virtual bool EncodePacket( void* payload, size_t len_of_byte ) override;
    virtual bool DecodePacket( void* packet, size_t len_of_byte ) override;
public://Transport
    virtual bool SendRtp( const uint8_t* packet,
                          size_t length,
                          const webrtc::PacketOptions& options ) override;
    virtual bool SendRtcp( const uint8_t* packet, size_t length ) override;
public://RtpData
    virtual int32_t OnReceivedPayloadData( const uint8_t* payloadData,
                                           const size_t payloadSize,
                                           const webrtc::WebRtcRTPHeader* rtpHeader ) override;

    virtual bool OnRecoveredPacket( const uint8_t* packet,
                                    size_t packet_length ) override;
public://RtcpBandwidthObserver
    virtual void OnReceivedEstimatedBitrate( uint32_t bitrate ) override;

    virtual void OnReceivedRtcpReceiverReport(
        const webrtc::ReportBlockList& report_blocks,
        int64_t rtt,
        int64_t now_ms ) override;
public://RtcpRttStats
    virtual void OnRttUpdate( int64_t rtt ) override;

    virtual int64_t LastProcessedRtt() const override;
public://BitrateStatisticsObserver
    virtual void Notify( const webrtc::BitrateStatistics& total_stats,
                         const webrtc::BitrateStatistics& retransmit_stats,
                         uint32_t ssrc );
protected:
private:
    webrtc::RtpRtcp*              rtp_rtcp_ = nullptr;
    webrtc::RtpReceiver*          rtp_receiver_ = nullptr;
    webrtc::RtpHeaderParser*      rtp_parse_ = nullptr;
    webrtc::RTPPayloadRegistry*   rtp_payload_registry_ = nullptr;
    webrtc::Clock*                clock_ = nullptr;
    webrtc::ReceiveStatistics*    receiver_stat_ = nullptr;
};