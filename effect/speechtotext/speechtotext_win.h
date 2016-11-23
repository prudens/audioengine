/*!
 * \file speechtotext_win.h
 *
 * \author zhangnaigan
 * \date 十月 2016
 *
 * 实现windows版本的语音转文字功能
 */
#pragma once
#ifdef _WIN32
#include <string>
#include "speechtotext.h"

class SpeechToTextWin:public SpeechToText
{
public:
    SpeechToTextWin(std::string appid);
    virtual ~SpeechToTextWin();
    int Write( const char* audioSample, std::size_t nSamples );
    int GetResult( std::string &strText ); // 如果string为空，则表示无法识别，原因包括，没联网，服务器不能用，或者语音太模糊，或者根本就没说话
    bool Start();
    void Cancel();
    void Finish();
    bool isStop() { return false; }
private:

    std::string grammar_id_;
    const char* session_id_ = nullptr;
    int 			aud_stat_;		//音频状态
    int 			ep_stat_;		//端点检测
    int 			rec_stat_;		//识别状态	
    int count = 0;
    std::string result_;
};

#endif//_WIN32