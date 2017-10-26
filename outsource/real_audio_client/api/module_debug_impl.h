#pragma once 
//#include "media_base_client/imedia_base_client.h"
#include "IModuleDebuger.h"
#include "snail_audio_engine_impl.h"


namespace snail{
namespace audio{
	   
    class ModuleDebugerImpl :public IModuleDebuger
{
public:
	ModuleDebugerImpl(AudioRoomImpl* host);
	~ModuleDebugerImpl();

	int RegisterEventHandler(IModuleDebugerHandler* handler);
	int EnableMonitorUserDelay(bool enable);
	int EnableMonitorNet(bool enable);
	int EnableSavePlayout(bool enable);
	int EnableSaveRecord(bool enable);
	int PlaySavedFile(int id, bool bPlay);
	int PlayPause(bool bPause);
private:
    AudioRoomImpl* _host = nullptr;
	IModuleDebugerHandler* _handler = nullptr;
    //snail::client::media::imedia_base_client* _media_base_client;
};
 
}}
