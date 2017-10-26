////仅供内部开发人员调试使用


#ifndef INTERFACE_MODULE_DEBUG_H
#define INTERFACE_MODULE_DEBUG_H
#pragma once

#include "ISnailAudioEngine.h"
 
namespace snail{namespace audio{

#define INNER_MODULE_DEBUGGER  151
 
#define TYPE_RECORD_DEV             0X01
#define TYPE_RECORD_WEBRTC          0X02
#define TYPE_RECORD_NS              0X04
#define TYPE_RECORD_VOICE_CHECK     0X08
#define TYPE_RECORD_BEFORE_ENCODE   0X10
#define TYPE_RECORD_SEND            0X20
#define TYPE_PLAYOUT_RECEIVE        0X40
#define TYPE_PLAYOUT_DECODE         0X80
#define TYPE_PLAYOUT_MIXED          0X100
#define TYPE_PLAYOUT_DEV            0X200

#define TYPE_RECORD_ALL  (TYPE_RECORD_DEV | TYPE_RECORD_WEBRTC | TYPE_RECORD_NS | TYPE_RECORD_VOICE_CHECK | TYPE_RECORD_BEFORE_ENCODE | TYPE_RECORD_SEND)
#define TYPE_PLAYOUT_ALL (TYPE_PLAYOUT_RECEIVE | TYPE_PLAYOUT_DECODE | TYPE_PLAYOUT_MIXED | TYPE_PLAYOUT_DEV)
#define TYPE_ALL         (TYPE_RECORD_ALL | TYPE_PLAYOUT_ALL)


class IModuleDebugerHandler
{
public: 
	virtual void NotifyShowMessage(const char* message) = 0;
};

class IModuleDebuger : public IModule
{
public:  
	virtual int RegisterEventHandler(IModuleDebugerHandler* handler) = 0;
	virtual int EnableMonitorUserDelay(bool enable) = 0;
	virtual int EnableMonitorNet(bool enable) = 0;
	virtual int EnableSavePlayout(bool enable) = 0;
	virtual int EnableSaveRecord(bool enable) = 0;
	virtual int PlaySavedFile(int type, bool bPlay) = 0;
	virtual int PlayPause(bool bPause) = 0;
};

 

}}//snail::audio
#endif//INTERFACE_MODULE_DEBUG_H
