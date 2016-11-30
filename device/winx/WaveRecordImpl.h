// _DeviceWaveRecordImpl.h: interface for the _DeviceWaveRecordImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__DEVICEWAVERECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_)
#define AFX__DEVICEWAVERECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "_DeviceRecoder.h"

class _DeviceWaveRecordImpl : public _DeviceRecoder  
{ 
public:
	_DeviceWaveRecordImpl();
	virtual ~_DeviceWaveRecordImpl();

	void   RegisterAudioCallback(_CallBack* callback, const void* userdata);
	bool   OpenDevice(int deviceindex, int channel, int samplerate, int frameSize);
	void   CloseDevice();


	static INT_PTR CALLBACK EnumCallback(GUID* pGUID, LPSTR szDesc, LPSTR szDrvName, void* pContext);
	int    GetDeviceNum();
	bool   GetDeviceName(int deviceindex, char devname[250]); 
	void   ThreadEntryPoint();
  

protected:
	const void*          m_userdata;
	_CallBack*			 m_callback;
	bool				 m_opened;
	int					 m_devindex;
	int					 m_channel;
	unsigned long        m_frameSize;
	int					 m_samplerate;
	int					 m_openstatus;
};

#endif // !defined(AFX__DEVICEWAVERECORDIMPL_H__6F1D7ED9_2660_4B60_9055_FF49F118FA7F__INCLUDED_)
