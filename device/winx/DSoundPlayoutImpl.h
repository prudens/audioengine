// _DeviceDSoundPlayoutImpl.h: interface for the _DeviceDSoundPlayoutImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__DEVICEDSOUNDPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_)
#define __DEVICEDSOUNDPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_DevicePlayout.h"
#include <vector>

class _DeviceDSoundPlayoutImpl : public _DevicePlayout    
{
	struct dev
	{
		GUID guid;
		char name[250];
	};
public:
	_DeviceDSoundPlayoutImpl();
	virtual ~_DeviceDSoundPlayoutImpl(); 
	
	void    RegisterAudioCallback(_CallBack* callback, const void* userdata);
	bool    OpenDevice(int deviceindex, int channel, int samplerate, int frameSize);
	void    CloseDevice();
	int     GetDeviceNum();
	bool    GetDeviceName(int deviceindex, char devname[250]); 
	void    ThreadEntryPoint();

protected:
	void    OpenDeviceForPlayout();
	static INT_PTR CALLBACK EnumCallback(GUID* pGUID, LPSTR szDesc, LPSTR szDrvName, void* pContext);


protected:
	std::vector<dev*>	 m_devs;
	_CallBack*			 m_callback;
	const void*          m_userdata;
	bool				 m_opened;
	GUID				 m_devguid;
	int					 m_channel;
	unsigned long        m_frameSize;
	int					 m_samplerate;
	int					 m_openstatus;
};

#endif // !defined(__DEVICEDSOUNDPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_)
