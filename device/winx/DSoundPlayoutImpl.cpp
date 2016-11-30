// _DeviceDSoundPlayoutImpl.cpp: implementation of the _DeviceDSoundPlayoutImpl class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>          
#include <process.h>           
#include <list>
#include <mmsystem.h>
#include <dsound.h>
#include "_DeviceDSoundPlayoutImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DEVICE_OPEN_ERROR     -1
#define DEVICE_OPEN_OPENING    0
#define DEVICE_OPEN_SUCCEED    1

#define _safe_delay		      40//   30
#define _min_delay            40//   40
#define _max_delay			  120

#define LOG(x)
 

static void ThreadStaticEntryPoint(void * p)  
{  
	_DeviceDSoundPlayoutImpl * pthx = (_DeviceDSoundPlayoutImpl*)p;     
	pthx->ThreadEntryPoint();    
	_endthread();                    
}  


_DeviceDSoundPlayoutImpl::_DeviceDSoundPlayoutImpl()
{ 
	m_callback		= 0;
	m_opened		= false;   
	m_channel		= 0;
	m_frameSize		= 0;
	m_samplerate	= 0;
	m_openstatus	= 0;
 	m_devguid		= GUID_NULL; 
	m_userdata      = nullptr;
}

_DeviceDSoundPlayoutImpl::~_DeviceDSoundPlayoutImpl()
{
	while(!m_devs.empty())
	{
		dev* d  = m_devs.back();
		delete d;
		m_devs.pop_back();
	} 
}
 

void _DeviceDSoundPlayoutImpl::RegisterAudioCallback(_CallBack* callback, const void* userdata)
{
	m_callback = callback;
	m_userdata = userdata;
}
 
bool _DeviceDSoundPlayoutImpl::OpenDevice(int deviceindex, int channel, int samplerate, int frameSize)
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

void _DeviceDSoundPlayoutImpl::CloseDevice()
{
	if(!m_opened)
		return;
	
	m_opened = false;
	while(DEVICE_OPEN_SUCCEED == m_openstatus)
		Sleep(50); 
}

void _DeviceDSoundPlayoutImpl::ThreadEntryPoint()
{ 
	while(m_opened && DEVICE_OPEN_OPENING == m_openstatus)
	{
		OpenDeviceForPlayout();
	}
} 



void _DeviceDSoundPlayoutImpl::OpenDeviceForPlayout()
{
	LPDIRECTSOUND		         pDS		  = NULL; 
	LPDIRECTSOUNDBUFFER          pDSB		  = NULL ;
	HRESULT                      hr           = S_OK;  
    LPDIRECTSOUNDBUFFER          pDSBPrimary  = NULL;
    DSBUFFERDESC                 dscbd; 

	WAVEFORMATEX wfx;
	wfx.cbSize					= 0;
	wfx.wFormatTag				= WAVE_FORMAT_PCM;
	wfx.nChannels				= m_channel;
	wfx.nSamplesPerSec			= m_samplerate;
	wfx.wBitsPerSample			= 16;
	wfx.nBlockAlign				= wfx.wBitsPerSample * m_channel / 8;
	wfx.nAvgBytesPerSec			= wfx.nBlockAlign * wfx.nSamplesPerSec; 

	int           framedelay    = __max(5, 1000*m_frameSize/wfx.nAvgBytesPerSec);  
	unsigned long dwBufferSize  = __max(2, (_max_delay / framedelay )) * m_frameSize + m_frameSize;  
	unsigned long safe_delay    = __max(2, (_safe_delay / framedelay)) * m_frameSize;
	unsigned long min_delay     = __max(2, (_min_delay / framedelay)) * m_frameSize;  
 

	HWND hWnd					= GetDesktopWindow();
 
 	if( FAILED( hr = DirectSoundCreate(&m_devguid, &pDS, NULL ) ) ) 
		goto err; 
 
	if( FAILED( hr = pDS->SetCooperativeLevel(hWnd,  DSSCL_PRIORITY) ) )
        goto err;  
 
    // Get the primary buffer  
    memset( &dscbd, 0, sizeof(DSBUFFERDESC) );
    dscbd.dwSize        = sizeof(DSBUFFERDESC);
    dscbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dscbd.dwBufferBytes = 0;
    dscbd.lpwfxFormat   = NULL; 
    if( FAILED( hr = pDS->CreateSoundBuffer( &dscbd, &pDSBPrimary, NULL ) ) )
       goto err;  
	 
    if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
        goto err;  
	
	if(pDSBPrimary) 
	{
		pDSBPrimary->Release();  
		pDSBPrimary = NULL;
	}

 
	dscbd.dwFlags =  DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 
		            | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS; 

	dscbd.dwBufferBytes   = dwBufferSize;
	dscbd.dwReserved      = 0;  
	dscbd.lpwfxFormat     = &wfx;
	 
  	hr = pDS->CreateSoundBuffer( &dscbd, &pDSB, NULL );
	if( FAILED( hr ))
	{ 
		dscbd.dwFlags &= ~DSBCAPS_LOCHARDWARE;
		dscbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
		hr = pDS->CreateSoundBuffer( &dscbd, &pDSB, NULL); 
		
		if( FAILED( hr ))
			goto err;  
	} 
 
	//fill silience
	{
		void*         plockedbuffer = NULL; 
		unsigned long dwlockedsize  = dwBufferSize;
		if( FAILED(pDSB->Lock(0, 0,&plockedbuffer, &dwlockedsize,NULL, NULL,  DSBLOCK_ENTIREBUFFER) ) )
			goto err;  
		
		memset( (BYTE*) plockedbuffer, 0, dwlockedsize);  
		pDSB->Unlock(plockedbuffer, dwlockedsize, NULL, 0 );
	}

	pDSB->SetCurrentPosition(0); 
	if( FAILED(pDSB->Play(0, 0, DSBPLAY_LOOPING)) )
		goto err;	 

	m_openstatus          = DEVICE_OPEN_SUCCEED;

	// play data
	{
		long          nNextOffset     = m_frameSize; 
		unsigned long dwCurDevPos     = 0;
		unsigned long dwCurDataPos    = 0; 

		unsigned long filldelay_ttl   = min_delay;
		bool          less_timepoint  = GetTickCount();         

		
		while(m_opened)
		{
			if(FAILED( hr = pDSB->GetCurrentPosition(&dwCurDevPos, &dwCurDataPos)))
				break;
			
			LONG datasize  = nNextOffset - dwCurDataPos;
			if(datasize < 0)
				datasize += dwBufferSize;

			if(datasize <= min_delay)
			{
				if(datasize < safe_delay) 
					filldelay_ttl  = __min(dwBufferSize - m_frameSize, filldelay_ttl + m_frameSize);  
				less_timepoint = GetTickCount();
			}
			else if(GetTickCount() - less_timepoint > 4000)
			{ 
				int oldttl = filldelay_ttl;

				filldelay_ttl  = __max(min_delay, filldelay_ttl - m_frameSize); 
				less_timepoint = GetTickCount(); 
			}


			if(datasize <= filldelay_ttl)
			{
				void*         pbCaptureData = NULL;
				unsigned long dwLockedSize  = 0;
				hr = pDSB->Lock(nNextOffset, m_frameSize,  &pbCaptureData, &dwLockedSize,  NULL, NULL, 0L );
 
				if( FAILED( hr ))
					break; 	

				if(DSBSTATUS_BUFFERLOST == hr)
				{
					while(m_opened)
					{
						hr = pDSB->Restore(); 
						if( hr == DSERR_BUFFERLOST )
							Sleep( 5 );
						else
							break;
					}

					LOG("buffer lost\n");

					hr = pDSB->Lock(nNextOffset, m_frameSize,  &pbCaptureData, &dwLockedSize,  NULL, NULL, 0L );

					if( FAILED( hr ))
						break; 
				}
  
				LONG delayLength  = nNextOffset - dwCurDevPos;
				if(delayLength < 0)
					delayLength += dwBufferSize;

				bool breset = false;
				int nReaded = m_callback->ReadPlayoutSamples(pbCaptureData, dwLockedSize, delayLength, breset, m_userdata);

				if(0 == nReaded) 
					memset(pbCaptureData, 0, m_frameSize); 
				
				pDSB->Unlock(pbCaptureData, dwLockedSize, NULL, NULL); 
				
				nNextOffset  += m_frameSize; 
				if(nNextOffset == dwBufferSize)
					nNextOffset = 0;
			}
			else
			{ 
				Sleep(framedelay/2); 
			}
		}   

	} 


	pDSB->Stop(); 

err:
  	if(pDSBPrimary)
 		pDSBPrimary->Release();

	if(pDSB)
		pDSB->Release();

	if(pDS)
		pDS->Release();


	m_openstatus = DEVICE_OPEN_ERROR;   
}


INT_PTR CALLBACK _DeviceDSoundPlayoutImpl::EnumCallback(GUID* pGUID, LPSTR szDesc, LPSTR szDrvName, void* pContext)
{
    GUID* pTemp  = NULL;
    _DeviceDSoundPlayoutImpl *pOwner = (_DeviceDSoundPlayoutImpl*)pContext;
	
    if( pGUID )
    {   
		dev* d  = new dev();
		d->guid = *pGUID;
		strcpy(d->name, szDesc); 
		pOwner->m_devs.push_back(d);
	}
	
    return TRUE;
}
 

int _DeviceDSoundPlayoutImpl::GetDeviceNum()
{ 
	while(!m_devs.empty())
	{
		dev* d  = m_devs.back();
		delete d;
		m_devs.pop_back();
	}
	
	if(FAILED(DirectSoundEnumerate( (LPDSENUMCALLBACK)EnumCallback, (void*)this )))
		return 0; 
	
	return m_devs.size();
}

bool _DeviceDSoundPlayoutImpl::GetDeviceName(int deviceindex, char devname[250])
{
	if(0 == m_devs.size())
		GetDeviceNum();

	int devnum = m_devs.size();
	if(deviceindex < 0 || deviceindex >= devnum)
		return false;

	strcpy(devname, m_devs[deviceindex]->name); 
	
	return true;
} 
