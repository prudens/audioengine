// _DeviceWavePlayoutImpl.h: interface for the _DeviceWavePlayoutImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__DEVICEWAVEPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_)
#define AFX__DEVICEWAVEPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_DevicePlayout.h"
 

class _DeviceWavePlayoutImpl : public _DevicePlayout    
{
public:
	_DeviceWavePlayoutImpl();
	virtual ~_DeviceWavePlayoutImpl(); 
	
	void    RegisterAudioCallback(_CallBack* callback, const void* userdata);
	bool    OpenDevice(int deviceindex, int channel, int samplerate, int frameSize);
	void    CloseDevice();
	int     GetDeviceNum();
	bool    GetDeviceName(int deviceindex, char devname[250]); 
	void    ThreadEntryPoint(); 

protected:
	void    OpenDeviceForPlayout();

protected:
	_CallBack*			 m_callback;
	const void*          m_userdata;
	bool				 m_opened;
	int					 m_devindex;
	int					 m_channel;
	unsigned long        m_frameSize;
	int					 m_samplerate;
	int					 m_openstatus;  
};

#endif // !defined(AFX__DEVICEWAVEPLAYOUTIMPL_H__797CF08F_455E_4876_B2E5_E12B2C7BEE38__INCLUDED_)
