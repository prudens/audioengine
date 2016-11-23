#pragma once
/*!
 * \file voicetotext.h
 *
 * \author zhangnaigan
 * \date 十月 2016
 *
 * \note 封装了科大讯飞的语音转文本功能，在线版
 */
#include <string>
#include <memory>
class SpeechToText
{
public:
    static SpeechToText* Create();
    virtual ~SpeechToText(){}
    virtual int Write( const char* audioSample, std::size_t nSamples ) = 0;
    virtual int GetResult(std::string &strText) = 0; // 如果string为空，则表示无法识别，原因包括，没联网，服务器不能用，或者语音太模糊，或者根本就没说话
    virtual bool Start() = 0;
    virtual void Cancel() = 0;
    virtual void Finish() = 0;
    virtual bool isStop() = 0;
};