// _DeviceDSoundRecordImpl.h: interface for the _DeviceDSoundRecordImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__DEVICEDSOUNDRECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_)
#define __DEVICEDSOUNDRECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_DeviceRecoder.h"
#include <vector>

class _DeviceDSoundRecordImpl : public _DeviceRecoder  
{
	struct dev
	{
		GUID guid;
		char name[250];
	};
public:
	_DeviceDSoundRecordImpl();
	virtual ~_DeviceDSoundRecordImpl();

	void   RegisterAudioCallback(_CallBack* callback, const void* userdata);
	bool   OpenDevice(int deviceindex, int channel, int samplerate, int frameSize);
	void   CloseDevice();
	int    GetDeviceNum();
	bool   GetDeviceName(int deviceindex, char devname[250]); 
	void   ThreadEntryPoint();
	static INT_PTR CALLBACK EnumCallback(GUID* pGUID, LPSTR szDesc, LPSTR szDrvName, void* pContext);
 

protected:
	std::vector<dev*>	 m_devs;
	_CallBack*			 m_callback;
	bool				 m_opened;
	GUID				 m_devguid;
	int					 m_channel;
	unsigned long        m_frameSize;
	int					 m_samplerate;
	int					 m_openstatus;
	const void*          m_userdata;
};

#endif // !defined(__DEVICEDSOUNDRECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_)
