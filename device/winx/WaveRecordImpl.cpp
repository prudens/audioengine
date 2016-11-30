// _DeviceWaveRecordImpl.cpp: implementation of the _DeviceWaveRecordImpl class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>          // for HANDLE   
#include <process.h>          // for _beginthread()    
#include <stdio.h>
#include <vector>
#include "_DeviceWaveRecordImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DEVICE_OPEN_ERROR      -1
#define DEVICE_OPEN_OPENING     0
#define DEVICE_OPEN_SUCCEED     1  
#define _max_delay			  200
  
static void ThreadStaticEntryPoint(void * p)  
{  
	_DeviceWaveRecordImpl * pthx = (_DeviceWaveRecordImpl*)p;     
	pthx->ThreadEntryPoint();    
	_endthread(); 
}  


_DeviceWaveRecordImpl::_DeviceWaveRecordImpl()
{
	m_callback		= 0;
	m_opened		= false;  
	m_devindex		= 0;
	m_channel		= 0;
	m_frameSize		= 0;
	m_samplerate	= 0;
	m_openstatus	= 0; 
	m_userdata = nullptr;
}

_DeviceWaveRecordImpl::~_DeviceWaveRecordImpl()
{

}
 

void _DeviceWaveRecordImpl::RegisterAudioCallback(_CallBack* callback, const void* userdata)
{
	m_callback = callback;
	m_userdata = userdata;
}
 
bool _DeviceWaveRecordImpl::OpenDevice(int deviceindex, int channel, int samplerate, int frameSize)
{
	if(m_opened)
		return false;

	m_opened     = true;  
	m_channel    = channel;
	m_samplerate = samplerate;
	m_frameSize  = frameSize;
	m_openstatus = DEVICE_OPEN_OPENING; 
 
    _beginthread(ThreadStaticEntryPoint, 0, this);   

	while(DEVICE_OPEN_OPENING == m_openstatus) 
		Sleep(50); 

	if(DEVICE_OPEN_SUCCEED == m_openstatus)
	{
		return true;
	}
	else
	{
		m_opened = false;
		return false;
	}
}
 
void _DeviceWaveRecordImpl::CloseDevice()
{
	if(!m_opened)
		return;

	m_opened = false;
	while(DEVICE_OPEN_SUCCEED == m_openstatus)
		Sleep(50); 
}

void _DeviceWaveRecordImpl::ThreadEntryPoint()
{  
	WAVEFORMATEX wfx;
	wfx.cbSize = 0;
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nChannels       = m_channel;
	wfx.nSamplesPerSec  = m_samplerate;
	wfx.wBitsPerSample  = 16;
	wfx.nAvgBytesPerSec = m_channel * m_samplerate * wfx.wBitsPerSample / 8;
	wfx.nBlockAlign     = wfx.wBitsPerSample * m_channel / 8;
	wfx.cbSize          = 0;

	
	HWAVEIN hWave;
	if(MMSYSERR_NOERROR != waveInOpen(&hWave, m_devindex, &wfx, GetCurrentThreadId(), 0, CALLBACK_THREAD))
	{
		m_openstatus = DEVICE_OPEN_ERROR;
		return;
	} 

 	int          initcount      = __max(2, _max_delay* wfx.nAvgBytesPerSec/1000/m_frameSize);  
	int          incount        = 0;
 	 
	std::vector<WAVEHDR*> wheaders; 
	for(int i = 0; i<initcount; i++)
	{
		WAVEHDR* wheader = (WAVEHDR*)malloc(sizeof(WAVEHDR));
		memset(wheader, 0, sizeof(WAVEHDR)); 
		wheaders.push_back(wheader); 
		
		wheader->lpData		     = (LPSTR)malloc(m_frameSize);
		memset(wheader->lpData, 0, m_frameSize);
		wheader->dwBufferLength  = m_frameSize; 
		wheader->dwBytesRecorded = 0;
		
		waveInPrepareHeader(hWave, wheader, sizeof(WAVEHDR));
		waveInAddBuffer(hWave, wheader, sizeof(WAVEHDR));
		incount ++; 
	} 


	if(MMSYSERR_NOERROR == waveInStart(hWave))
	{
		m_openstatus = DEVICE_OPEN_SUCCEED;

		MSG msg;
		while(GetMessage(&msg, 0, 0, 0))
		{
			if(MM_WIM_DATA == msg.message)
			{ 
				WAVEHDR* wheader = (WAVEHDR*)msg.lParam;
				waveInUnprepareHeader(hWave, wheader, sizeof(WAVEHDR));
				incount --;  
  
				if(m_opened) 
				{   
 					bool bReset = false; 
					if(wheader->dwBytesRecorded == m_frameSize)
						m_callback->RecordedSamples(wheader->lpData, wheader->dwBytesRecorded, bReset, m_userdata);

 					waveInPrepareHeader(hWave, wheader, sizeof(WAVEHDR));
					waveInAddBuffer(hWave, wheader, sizeof(WAVEHDR));
					incount ++;
				}  
				else
				{
					if(0 == incount)
					{
						waveInClose(hWave);
						break;
					}
				}
			}
			else if(MM_WIM_CLOSE == msg.message)
			{ 
				break;
			}
		}  
	}  

	while(!wheaders.empty())
	{
		WAVEHDR* wheader = wheaders.back();
		wheaders.pop_back();

		free(wheader->lpData);
		free(wheader);
	} 

	m_openstatus = DEVICE_OPEN_ERROR;   
}

int _DeviceWaveRecordImpl::GetDeviceNum()
{ 
	return waveInGetNumDevs();
}
 
bool _DeviceWaveRecordImpl::GetDeviceName(int deviceindex, char devname[250])
{ 
	int devnum = GetDeviceNum();
	if(deviceindex < 0 || deviceindex >= devnum)
		return false;
	
	WAVEINCAPSW caps;
	waveInGetDevCapsW(deviceindex, &caps, sizeof(WAVEINCAPSW)); 
	
	
	devname[0] = 0;
	WideCharToMultiByte(CP_ACP, 0, caps.szPname, -1, devname, 250, NULL, NULL); 
	
	return true;
}
 