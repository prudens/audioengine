
#include <memory>
//#include "system/media_error_code.h"
//#include "system/system.h"
#include "module_debug_impl.h"
#include "audio_typedef.h" 

//#include <audio_engine/IAudio_Module.h>
//#include <media_audio_realtime/imedia_audio_realtime.h>
namespace audio_engine{
	//using namespace snail::client::media;
	//using namespace snail::client::media::audio;

 
	ModuleDebugerImpl::ModuleDebugerImpl(AudioRoomImpl* host)
    {
        _host = host;
        //_media_base_client = host->_media_base_client.get();
    }

	ModuleDebugerImpl::~ModuleDebugerImpl()
    {

    }

	int ModuleDebugerImpl::RegisterEventHandler(IModuleDebugerHandler * handler)
    {
        if (!handler)
        {
            return ERR_INVALID_ARGUMENT;
        }
        _handler = handler; 
        return ERR_OK;
    }

	int ModuleDebugerImpl::EnableMonitorUserDelay(bool enable)
	{
		/*IMedia_Client* pModule = (IMedia_Client*)_media_base_client->IO_GetModule(module_media_client); 
		if (nullptr == pModule)
			return ERR_NOT_SUPPORTED;

		if (nullptr == _handler)
			return ERR_NOT_INITIALIZE;

		pModule->IO_SetAudioDataDelay([=](const std::string& szUserid, int nDelayTime)
		{
			std::string speakerid = szUserid;
			auto handle = [=]() { 
				std::string message = snail::tools::CTools::FormatString("%s delay: %d", speakerid.c_str(), nDelayTime); 
				_handler->NotifyShowMessage(message.c_str());
			}; 
			_host->RecvAsyncEvent(std::move(handle));
		}); */

		return ERR_OK;
	}


	int ModuleDebugerImpl::EnableMonitorNet(bool enable)
	{
		//IMedia_Client* pModule = (IMedia_Client*)_media_base_client->IO_GetModule(module_media_client);
		//if (nullptr == pModule)
		//	return ERR_NOT_SUPPORTED;

		//if (nullptr == _handler)
		//	return ERR_NOT_INITIALIZE;

		//pModule->IO_SetNetWatcher([=](int type, const std::string& message)
		//{
		//	std::string showmsg = message;
		//	auto handle = [=]() { 
		//		_handler->NotifyShowMessage(showmsg.c_str());
		//	};
		//	_host->RecvAsyncEvent(std::move(handle));
		//});

		return ERR_OK;
	}

	int ModuleDebugerImpl::EnableSavePlayout(bool bSave)
	{
		//IAudio_Module* pModule = (IAudio_Module*)_media_base_client->IO_GetModule(module_audio);

		//if (nullptr == pModule)
		//	return ERR_NOT_SUPPORTED;

		//IAudio_Tester* pTester = (IAudio_Tester*)pModule->IO_GetInner(AUDIO_INNER_TYPE_TESTER);
		// 
		//if (nullptr == pTester)
		//	return ERR_NOT_SUPPORTED;

		//pTester->IO_Save(IAudio_Tester::type_playout_all, bSave);

		return ERR_OK;
	}

	int ModuleDebugerImpl::EnableSaveRecord(bool bSave)
	{
		//IAudio_Module* pModule = (IAudio_Module*)_media_base_client->IO_GetModule(module_audio);

		//if (nullptr == pModule)
		//	return ERR_NOT_SUPPORTED;

		//IAudio_Tester* pTester = (IAudio_Tester*)pModule->IO_GetInner(AUDIO_INNER_TYPE_TESTER);

		//if (nullptr == pTester)
		//	return ERR_NOT_SUPPORTED;

		//pTester->IO_Save(IAudio_Tester::type_record_all, bSave);

		return ERR_OK;
	}

	int ModuleDebugerImpl::PlaySavedFile(int id, bool bPlay)
	{
		//IAudio_Module* pModule = (IAudio_Module*)_media_base_client->IO_GetModule(module_audio);

		//if (nullptr == pModule)
		//	return ERR_NOT_SUPPORTED;

		//IAudio_Tester* pTester = (IAudio_Tester*)pModule->IO_GetInner(AUDIO_INNER_TYPE_TESTER);

		//if (nullptr == pTester)
		//	return ERR_NOT_SUPPORTED;

		//pTester->IO_Play((IAudio_Tester::eType)id, bPlay, true);

		return ERR_OK;
	}

	int ModuleDebugerImpl::PlayPause(bool bPause)
	{
		//IAudio_Module* pModule = (IAudio_Module*)_media_base_client->IO_GetModule(module_audio);

		//if (nullptr == pModule)
		//	return ERR_NOT_SUPPORTED;

		//IAudio_Tester* pTester = (IAudio_Tester*)pModule->IO_GetInner(AUDIO_INNER_TYPE_TESTER);

		//if (nullptr == pTester)
		//	return ERR_NOT_SUPPORTED;

		//pTester->IO_PausePlay(bPause);

		return ERR_OK;
	} 


}