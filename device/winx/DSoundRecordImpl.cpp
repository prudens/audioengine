// _DeviceDSoundRecordImpl.cpp: implementation of the _DeviceDSoundRecordImpl class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>          // for HANDLE   
#include <process.h>          // for _beginthread()    
#include <stdio.h>
#include <vector>
#include <mmsystem.h>
#include <dsound.h>
#include "_DeviceDSoundRecordImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////




#define DEVICE_OPEN_ERROR      -1
#define DEVICE_OPEN_OPENING     0
#define DEVICE_OPEN_SUCCEED     1  
#define _max_delay			  200 

static void ThreadStaticEntryPoint(void * p)  
{  
	_DeviceDSoundRecordImpl * pthx = (_DeviceDSoundRecordImpl*)p;     
	pthx->ThreadEntryPoint();    
	_endthread(); 
}  


_DeviceDSoundRecordImpl::_DeviceDSoundRecordImpl()
{
	m_callback		= 0;
	m_opened		= false;  
	m_devguid		= GUID_NULL;
	m_channel		= 0;
	m_frameSize		= 0;
	m_samplerate	= 0;
	m_openstatus	= 0;
	m_userdata = nullptr;
}

_DeviceDSoundRecordImpl::~_DeviceDSoundRecordImpl()
{
	while(!m_devs.empty())
	{
		dev* d  = m_devs.back();
		delete d;
		m_devs.pop_back();
	} 
}
 

void _DeviceDSoundRecordImpl::RegisterAudioCallback(_CallBack* callback, const void* userdata)
{
	m_callback = callback;
	m_userdata = userdata;
}
 
bool _DeviceDSoundRecordImpl::OpenDevice(int deviceindex, int channel, int samplerate, int frameSize)
{
	if(m_opened)
		return false;

	if(-1 == deviceindex)
	{
		m_devguid    = GUID_NULL;
	}
	else
	{
		if(0 == m_devs.size())
			GetDeviceNum();

		if(deviceindex >= m_devs.size() || deviceindex < -1)
			return false;

		m_devguid = m_devs[deviceindex]->guid;
	}

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
 
void _DeviceDSoundRecordImpl::CloseDevice()
{
	if(!m_opened)
		return;

	m_opened = false;
	while(DEVICE_OPEN_SUCCEED == m_openstatus)
		Sleep(50); 
}

void _DeviceDSoundRecordImpl::ThreadEntryPoint()
{    
	LPDIRECTSOUNDCAPTURE         pDS	      = NULL; 
	LPDIRECTSOUNDCAPTUREBUFFER   pDSB		  = NULL ;
	HRESULT                      hr           = S_OK;  
    DSCBUFFERDESC                dscbd; 

	WAVEFORMATEX wfx;
	wfx.cbSize					= 0;
	wfx.wFormatTag				= WAVE_FORMAT_PCM;
	wfx.nChannels				= m_channel;
	wfx.nSamplesPerSec			= m_samplerate;
	wfx.wBitsPerSample			= 16;
	wfx.nBlockAlign				= wfx.wBitsPerSample * m_channel / 8;
	wfx.nAvgBytesPerSec			= wfx.nBlockAlign * wfx.nSamplesPerSec;  
	int           framedelay    = __max(5, 1000*m_frameSize/wfx.nAvgBytesPerSec);  
	unsigned long dwBufferSize  = __max(2, (_max_delay / framedelay )) * m_frameSize; 
    
 	if( FAILED( hr = DirectSoundCaptureCreate(&m_devguid, &pDS, NULL ) ) ) 
		goto err; 
	
	// Create the capture buffer
	memset( &dscbd, 0, sizeof(dscbd) );
	dscbd.dwSize        = sizeof(dscbd);
	dscbd.dwBufferBytes = dwBufferSize;
	dscbd.lpwfxFormat   = &wfx;  
	
	 
	if( FAILED( hr = pDS->CreateCaptureBuffer( &dscbd, &pDSB, NULL ) ) ) 
		goto err; 
 
	  
    if( FAILED( hr = pDSB->Start( DSCBSTART_LOOPING ) ) )
		goto err; 
	 

	m_openstatus          = DEVICE_OPEN_SUCCEED;

	// capture data
	{
		long          nNextOffset     = 0; 
		unsigned long dwCurDevPos     = 0;
		unsigned long dwCurDataPos    = 0; 
		
		while(m_opened)
		{
			if(FAILED( hr = pDSB->GetCurrentPosition(&dwCurDevPos, &dwCurDataPos)))
				break;
			
			LONG datasize  = dwCurDataPos - nNextOffset;
			if(datasize < 0)
				datasize += dwBufferSize;
 
			while(datasize > m_frameSize)
			{
				void*         pbCaptureData = NULL;
				unsigned long dwLockedSize  = 0;
				hr = pDSB->Lock(nNextOffset, m_frameSize, 
					&pbCaptureData, &dwLockedSize, 
					NULL, NULL, 0L );
				
				if( FAILED( hr ))
					break; 		
				
				bool breset = false;
				m_callback->RecordedSamples(pbCaptureData, dwLockedSize, breset, m_userdata);
				
				pDSB->Unlock(pbCaptureData, dwLockedSize, NULL, NULL);
				datasize -= m_frameSize;

				//computer delay
				{
					LONG delayLength = dwCurDevPos - nNextOffset; 
					if(delayLength < 0)
						delayLength += dwBufferSize; 
//					m_delay = 1000*delayLength/wfx.nAvgBytesPerSec; 
				}

				
				nNextOffset += m_frameSize;
				if(nNextOffset == dwBufferSize)
					nNextOffset = 0;
			}
			
			Sleep(framedelay/2); 
		}   

	} 


	pDSB->Stop(); 

err:  
	if(pDSB)
		pDSB->Release();

	if(pDS)
		pDS->Release();


	m_openstatus = DEVICE_OPEN_ERROR;   
}
 

INT_PTR CALLBACK _DeviceDSoundRecordImpl::EnumCallback(GUID* pGUID, LPSTR szDesc, LPSTR szDrvName, void* pContext)
{
    GUID* pTemp  = NULL;
    _DeviceDSoundRecordImpl *pOwner = (_DeviceDSoundRecordImpl*)pContext;
	
    if( pGUID )
    {   
		dev* d  = new dev();
		d->guid = *pGUID;
		strcpy(d->name, szDesc); 
		pOwner->m_devs.push_back(d);
	}
	
    return TRUE;
}
 

int _DeviceDSoundRecordImpl::GetDeviceNum()
{ 
	while(!m_devs.empty())
	{
		dev* d  = m_devs.back();
		delete d;
		m_devs.pop_back();
	}
 
	if(FAILED(DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)EnumCallback, (void*)this )))
		return 0; 
	
	return m_devs.size();
}
 
bool _DeviceDSoundRecordImpl::GetDeviceName(int deviceindex, char devname[250])
{
	if(0 == m_devs.size())
		GetDeviceNum();

	int devnum = m_devs.size();
	if(deviceindex < 0 || deviceindex >= devnum)
		return false;

	strcpy(devname, m_devs[deviceindex]->name); 
	
	return true;
}
