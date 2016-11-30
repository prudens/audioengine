// _DeviceWavePlayoutImpl.cpp: implementation of the _DeviceWavePlayoutImpl class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>          
#include <process.h>           
#include <list>
#include "_DeviceWavePlayoutImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DEVICE_OPEN_ERROR     -1
#define DEVICE_OPEN_OPENING    0
#define DEVICE_OPEN_SUCCEED    1

#define _safe_delay		  60//40//   30
#define _min_delay        50//   40
#define _max_delay		 120// 100
 

static void ThreadStaticEntryPoint(void * p)  
{  
	_DeviceWavePlayoutImpl * pthx = (_DeviceWavePlayoutImpl*)p;     
	pthx->ThreadEntryPoint();    
	_endthread();                    
}  


_DeviceWavePlayoutImpl::_DeviceWavePlayoutImpl()
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

_DeviceWavePlayoutImpl::~_DeviceWavePlayoutImpl()
{

}
 

void _DeviceWavePlayoutImpl::RegisterAudioCallback(_CallBack* callback, const void* userdata)
{
	m_callback = callback;
	m_userdata = userdata;
}
 
bool _DeviceWavePlayoutImpl::OpenDevice(int deviceindex, int channel, int samplerate, int frameSize)
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

void _DeviceWavePlayoutImpl::CloseDevice()
{
	if(!m_opened)
		return;
	
	m_opened = false;
	while(DEVICE_OPEN_SUCCEED == m_openstatus)
		Sleep(50); 
}

void _DeviceWavePlayoutImpl::ThreadEntryPoint()
{ 
	while(m_opened && DEVICE_OPEN_OPENING == m_openstatus)
	{
		OpenDeviceForPlayout();
	}
} 


void _DeviceWavePlayoutImpl::OpenDeviceForPlayout()
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
	    
	HWAVEOUT hWave;
	if(MMSYSERR_NOERROR != waveOutOpen(&hWave, m_devindex, &wfx, GetCurrentThreadId(), 0, CALLBACK_THREAD))
	{
		m_openstatus = DEVICE_OPEN_ERROR;
		return;
	}
 
	unsigned long moredelaytime   = 0;
	unsigned int  szFilled		  = 0; 
	int           incount		  = 0; 
	int           initNum         = __max(2, _min_delay* wfx.nAvgBytesPerSec/1000/m_frameSize); 
	int           maxcount        = _max_delay * wfx.nAvgBytesPerSec/1000/m_frameSize; 
 
	std::list<WAVEHDR*> wheaders; 
 	for(int i = 0; i<initNum; i++)
	{
		WAVEHDR* wheader = (WAVEHDR*)malloc(sizeof(WAVEHDR));
		memset(wheader, 0, sizeof(WAVEHDR)); 
	    wheaders.push_back(wheader); 

		wheader->lpData		     = (LPSTR)malloc(m_frameSize);
		memset(wheader->lpData, 0, m_frameSize);
		wheader->dwBufferLength  = m_frameSize; 

		waveOutPrepareHeader(hWave, wheader, sizeof(WAVEHDR)); 
		waveOutWrite(hWave, wheader, sizeof(WAVEHDR));
		incount ++;
		szFilled += wheader->dwBufferLength;
	} 

	m_openstatus      = DEVICE_OPEN_SUCCEED; 

	MSG msg;
	while(GetMessage(&msg, 0, 0, 0))
	{
		if(MM_WOM_DONE == msg.message)
		{ 
			WAVEHDR* wheader = (WAVEHDR*)msg.lParam;
			waveOutUnprepareHeader(hWave, wheader, sizeof(WAVEHDR)); 
			incount --; 
			
			if(m_opened && DEVICE_OPEN_SUCCEED == m_openstatus) 
			{   
				MMTIME mt;
				mt.wType = TIME_BYTES;  
				waveOutGetPosition(hWave, &mt, sizeof(MMTIME)); 
				
				unsigned int delayLength = szFilled - mt.u.cb;
				unsigned int delay       = 1000*delayLength/wfx.nAvgBytesPerSec; 

				int doTimes = 0;
				if(incount >= 3 && delay > _min_delay)
				{
					if(0 == moredelaytime)
					{
						moredelaytime = GetTickCount();
						doTimes   = 1;
					}
					else if(GetTickCount() - moredelaytime >= 2000)
					{ 
						moredelaytime = GetTickCount();
						doTimes   = 0; 
						
						for(std::list<WAVEHDR*>::iterator it = wheaders.begin(); it != wheaders.end(); it++) 
						{
							if(wheader == *it)
							{  
								wheaders.erase(it); 
								free(wheader->lpData);
								free(wheader);
								break;
							} 
						}  
					}
					else
					{
						doTimes   = 1;
					}
				}
				else
				{
					moredelaytime = 0;
					doTimes       = (delay < _safe_delay && incount < maxcount) ? 2 : 1;
				}
				
				
				while(doTimes > 0) 
				{
					wheader->dwBufferLength = m_frameSize; 
					bool bReset		    	= false;  
				    int  nReaded            = m_callback->ReadPlayoutSamples(wheader->lpData, wheader->dwBufferLength, delayLength, bReset, m_userdata);
 					
					if(bReset)
					{ 
					 	waveOutReset(hWave); 
						delay		 = 0;
						szFilled	 = 0;  
						m_openstatus = DEVICE_OPEN_OPENING;
						break;
					}
					
					if(0 == nReaded)
					{
						wheader->dwBufferLength = m_frameSize;
						memset(wheader->lpData, 0, m_frameSize);
					}
					else
					{
						delayLength += nReaded; 
					}
					
					szFilled += wheader->dwBufferLength;
					wheader->reserved = 0;
					waveOutPrepareHeader(hWave, wheader, sizeof(WAVEHDR));
					waveOutWrite(hWave, wheader, sizeof(WAVEHDR));
					incount ++; 
					doTimes --;
					
					if(doTimes > 0) 
					{
						wheader = (WAVEHDR*)malloc(sizeof(WAVEHDR));
						memset(wheader, 0, sizeof(WAVEHDR)); 
						wheaders.push_back(wheader);  
						wheader->lpData		     = (LPSTR)malloc(m_frameSize);
						memset(wheader->lpData, 0, m_frameSize);
						wheader->dwBufferLength  = m_frameSize;  
					}  
				} 
				
			}  
			else
			{
				if(0 == incount) 
					waveOutClose(hWave);  
			} 
		}
		else if(MM_WOM_CLOSE == msg.message)
		{ 
			break;
		}
	} 
 
	while(!wheaders.empty())
	{
		WAVEHDR* wheader = wheaders.back();
		wheaders.pop_back();
		
		free(wheader->lpData);
		free(wheader);
	} 
  
	if(DEVICE_OPEN_SUCCEED == m_openstatus)
		m_openstatus = DEVICE_OPEN_ERROR;    
}


int _DeviceWavePlayoutImpl::GetDeviceNum()
{ 
	return waveOutGetNumDevs();
}

bool _DeviceWavePlayoutImpl::GetDeviceName(int deviceindex, char devname[250])
{
	int devnum = GetDeviceNum();
	if(deviceindex < 0 || deviceindex >= devnum)
		return false;
	
	WAVEOUTCAPSW caps;
	waveOutGetDevCapsW(deviceindex, &caps, sizeof(WAVEINCAPSW)); 
	 
	devname[0] = 0;
	WideCharToMultiByte(CP_ACP, 0, caps.szPname, -1, devname, 250, NULL, NULL); 
	
	return true;
}
 