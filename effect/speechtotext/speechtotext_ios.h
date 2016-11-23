#pragma once

#include "speechtotext.h"
#ifdef _IOS 


class SpeechToTextIOSImpl;
class SpeechToTextIOS:public SpeechToText
{
public:
    SpeechToTextIOS(std::string appid);
    ~SpeechToTextIOS();
    virtual int Write( const char* audioSample, std::size_t nSamples );
    virtual int GetResult(std::string &strText);
    virtual bool Start();
    virtual void Finish();
    virtual bool isStop();
    virtual void Cancel();
private:
    SpeechToTextIOSImpl* impl_;
};

#endif //_IOS