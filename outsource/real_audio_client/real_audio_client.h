#pragma once
#include <cstdint>
#include "real_audio_common.h"
struct RAMsg;

class RealAudioClient
{

public:
    RealAudioClient();
    ~RealAudioClient();
public:
    int Start(std::string userid, RID roomid);
    int Stop();
    int Login();
    int Logout();
    int Speak( bool bEnable );
    int Playout( bool bEnable );
    int Speaking( bool &bEnable );
    int Playing( bool &bEnable );
    int GetMessage(RAMsg *msg);
public:
private:
    std::string userid;
    RID roomid = 0;
    
};