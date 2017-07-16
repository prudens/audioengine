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
    int Start(UID userid, RID roomid);
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
    UID userid = 0;
    RID roomid = 0;
    
};