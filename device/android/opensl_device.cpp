#include "device/android/opensl_device.h"
#include <stdio.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "AndroidProject1.NativeActivity", __VA_ARGS__))

AudioDevice* AudioDevice::Create()
{
    return new OpenSLDevice();
}

OpenSLDevice::OpenSLDevice()
{
    m_bInitialize = false;
    m_bInitPlayout = false;
    m_bInitRecording = false;
    m_bPlaying = false;
    m_bRecording = false;
    m_recSampleRate = 48000;
    m_plySampleRate = 48000;
    m_recChannel = 2;
    m_plyChannel = 2;
    m_engineObject = NULL;
    m_engineEngine = NULL;
    m_outputMixObject = NULL;
    m_plyObject = NULL;
    m_plyBufferQueue = NULL;

    m_plyPlayer = NULL;
    m_recObject = NULL;
    m_recRecorder = NULL;
    m_recBufferQueue = NULL;
    m_currentOutputBuffer = 0;
    m_currentInputIndex = 0;
    currentOutputIndex = 0;
    m_currentInputBuffer = 0;
}

OpenSLDevice::~OpenSLDevice()
{
    Terminate();
}

void OpenSLDevice::Release()
{
    delete this;
}

#define MAX_NUMBER_INTERFACES 2
bool OpenSLDevice::Initialize()
{
    if (m_bInitialize)
    {
        return true;
    }

    SLboolean required[MAX_NUMBER_INTERFACES];
    SLInterfaceID iidArray[MAX_NUMBER_INTERFACES];

    /* Initialize arrays required[] and iidArray[] */
    for ( int i = 0; i < MAX_NUMBER_INTERFACES; i++ )
    {
        required[i] = SL_BOOLEAN_FALSE;
        iidArray[i] = SL_IID_NULL;
    }

    iidArray[0] = SL_IID_ANDROIDEFFECTCAPABILITIES;
    required[0] = SL_BOOLEAN_TRUE;
    iidArray[1] = SL_IID_AUDIOIODEVICECAPABILITIES;
    required[1] = SL_BOOLEAN_FALSE;
    // 创建OpenSL引擎
    SLresult result;
    // create engine
    result = slCreateEngine( &m_engineObject, 0, NULL, MAX_NUMBER_INTERFACES, iidArray, required );
    if ( result != SL_RESULT_SUCCESS ) return  result;

    // realize the engine 
    result = ( *m_engineObject )->Realize( m_engineObject, SL_BOOLEAN_FALSE );
    if ( result != SL_RESULT_SUCCESS ) return result;

    // get the engine interface, which is needed in order to create other objects
    result = ( *m_engineObject )->GetInterface( m_engineObject, SL_IID_ENGINE, &m_engineEngine );
    if ( result != SL_RESULT_SUCCESS ) return result;




    // for test
    SLuint32 numinterfaces;
    result = slQueryNumSupportedEngineInterfaces( &numinterfaces );
    if (result == SL_RESULT_SUCCESS)
    {
        LOGW( "slQueryNumSupportedEngineInterfaces:%d",numinterfaces );
    }

    // 查询总共有多少采集设备
    SLAudioIODeviceCapabilitiesItf AudioIODeviceCapabilitiesItf;
    result = ( *m_engineObject )->GetInterface( m_engineObject, SL_IID_AUDIOIODEVICECAPABILITIES, &AudioIODeviceCapabilitiesItf );
    if ( result == SL_RESULT_SUCCESS )
    {
        SLuint32 InputDeviceIDs[20];
        SLint32 numInputs = 0;

       
        result = ( *AudioIODeviceCapabilitiesItf )->GetAvailableAudioInputs(
            AudioIODeviceCapabilitiesItf, &numInputs, InputDeviceIDs );
        if ( result == SL_RESULT_SUCCESS )
        {
            SLAudioInputDescriptor AudioInputDescriptor;
            SLuint32 mic_deviceID = 0;
            SLboolean mic_available = SL_BOOLEAN_FALSE;

            for ( int i = 0; i < numInputs; i++ )
            {
                LOGW( "AudioInputDescriptor :%d", i );
                result = ( *AudioIODeviceCapabilitiesItf )->QueryAudioInputCapabilities(
                    AudioIODeviceCapabilitiesItf, InputDeviceIDs[i],
                    &AudioInputDescriptor );
                if ( ( AudioInputDescriptor.deviceConnection
                       == SL_DEVCONNECTION_ATTACHED_WIRED )
                     && ( AudioInputDescriptor.deviceScope == SL_DEVSCOPE_USER )
                     && ( AudioInputDescriptor.deviceLocation
                          == SL_DEVLOCATION_HEADSET ) )
                {
                    mic_deviceID = InputDeviceIDs[i];
                    mic_available = SL_BOOLEAN_TRUE;
                    //break;
                }
                else if ( ( AudioInputDescriptor.deviceConnection
                            == SL_DEVCONNECTION_INTEGRATED )
                          && ( AudioInputDescriptor.deviceScope == SL_DEVSCOPE_USER )
                          && ( AudioInputDescriptor.deviceLocation
                               == SL_DEVLOCATION_HANDSET ) )
                {
                    mic_deviceID = InputDeviceIDs[i];
                    mic_available = SL_BOOLEAN_TRUE;
                    //break;
                }
            }
        }

    }
    m_bInitialize = true;
    return true;
}

void OpenSLDevice::Terminate()
{
    if (!m_bInitialize)
    {
        return;
    }

    StopPlayout();
    StopRecording();

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if ( m_plyObject != NULL )
    {
        ( *m_plyObject )->Destroy( m_plyObject );
        m_plyObject = NULL;
        m_plyPlayer = NULL;
        m_plyBufferQueue = NULL;
    }

    // destroy audio recorder object, and invalidate all associated interfaces
    if ( m_recObject != NULL )
    {
        ( *m_recObject )->Destroy( m_recObject );
        m_recObject = NULL;
        m_recRecorder = NULL;
        m_recBufferQueue = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if ( m_outputMixObject != NULL )
    {
        ( *m_outputMixObject )->Destroy( m_outputMixObject );
        m_outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if ( m_engineObject != NULL )
    {
        ( *m_engineObject )->Destroy( m_engineObject );
        m_engineObject = NULL;
        m_engineEngine = NULL;
    }
    m_bInitialize = false;
}

size_t OpenSLDevice::GetRecordingDeviceNum() const
{
    return 1;
}

size_t OpenSLDevice::GetPlayoutDeviceNum() const
{
    return 1;
}

bool OpenSLDevice::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}

bool OpenSLDevice::RecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}

bool OpenSLDevice::SetPlayoutDevice( int16_t index )
{
    return false;
}

bool OpenSLDevice::SetRecordingDevice( int16_t index )
{
    return false;
}

bool OpenSLDevice::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    SLuint32 SLSampleRate;
    if ( nChannels < 1 || nChannels > 2 )
    {
        return false;
    }

    switch ( nSampleRate )
    {
    case 8000:
        SLSampleRate = SL_SAMPLINGRATE_8;
        break;
    case 11025:
        SLSampleRate = SL_SAMPLINGRATE_11_025;
        break;
    case 16000:
        SLSampleRate = SL_SAMPLINGRATE_16;
        break;
    case 22050:
        SLSampleRate = SL_SAMPLINGRATE_22_05;
        break;
    case 24000:
        SLSampleRate = SL_SAMPLINGRATE_24;
        break;
    case 32000:
        SLSampleRate = SL_SAMPLINGRATE_32;
        break;
    case 44100:
        SLSampleRate = SL_SAMPLINGRATE_44_1;
        break;
    case 48000:
        SLSampleRate = SL_SAMPLINGRATE_48;
        break;
    case 64000:
        SLSampleRate = SL_SAMPLINGRATE_64;
        break;
    case 88200:
        SLSampleRate = SL_SAMPLINGRATE_88_2;
        break;
    case 96000:
        SLSampleRate = SL_SAMPLINGRATE_96;
        break;
    case 192000:
        SLSampleRate = SL_SAMPLINGRATE_192;
        break;
    default:
        return false;
    }

    SLresult result;
    // configure audio source
    SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };

    SLDataSource audioSrc = { &loc_dev, NULL };

    // configure audio sink
    SLuint32 speakers = nChannels > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;

    SLDataLocator_AndroidSimpleBufferQueue loc_bq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, kNumBuffer };
    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, nChannels, SLSampleRate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        speakers, SL_BYTEORDER_LITTLEENDIAN };
    SLDataSink audioSnk = { &loc_bq, &format_pcm };

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    SLObjectItf recorderObject;
    result = ( *m_engineEngine )->CreateAudioRecorder( m_engineEngine, &recorderObject, &audioSrc,
                                                     &audioSnk, 0, nullptr, nullptr );
    if ( recorderObject )
    {
        ( *recorderObject )->Destroy( recorderObject );
        recorderObject = nullptr;
    }
    return SL_RESULT_SUCCESS != result;
}

bool OpenSLDevice::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    SLuint32 SLSampleRate;
    if ( nChannels < 1 || nChannels > 2 )
    {
        return false;
    }

    switch ( nSampleRate )
    {
    case 8000:
        SLSampleRate = SL_SAMPLINGRATE_8;
        break;
    case 11025:
        SLSampleRate = SL_SAMPLINGRATE_11_025;
        break;
    case 16000:
        SLSampleRate = SL_SAMPLINGRATE_16;
        break;
    case 22050:
        SLSampleRate = SL_SAMPLINGRATE_22_05;
        break;
    case 24000:
        SLSampleRate = SL_SAMPLINGRATE_24;
        break;
    case 32000:
        SLSampleRate = SL_SAMPLINGRATE_32;
        break;
    case 44100:
        SLSampleRate = SL_SAMPLINGRATE_44_1;
        break;
    case 48000:
        SLSampleRate = SL_SAMPLINGRATE_48;
        break;
    case 64000:
        SLSampleRate = SL_SAMPLINGRATE_64;
        break;
    case 88200:
        SLSampleRate = SL_SAMPLINGRATE_88_2;
        break;
    case 96000:
        SLSampleRate = SL_SAMPLINGRATE_96;
        break;
    case 192000:
        SLSampleRate = SL_SAMPLINGRATE_192;
        break;
    default:
        return false;
    }
    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, kNumBuffer };
    const SLInterfaceID ids[] = { SL_IID_VOLUME };
    const SLboolean req[] = { SL_BOOLEAN_FALSE };
    result = ( *m_engineEngine )->CreateOutputMix( m_engineEngine, &m_outputMixObject, 1, ids, req );
    if ( result != SL_RESULT_SUCCESS )
        return result;

    // realize the output mix
    result = ( *m_outputMixObject )->Realize( m_outputMixObject, SL_BOOLEAN_FALSE );

    SLuint32 speakers;
    speakers = nChannels > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;

    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM,nChannels, nSampleRate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        speakers, SL_BYTEORDER_LITTLEENDIAN };

    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, m_outputMixObject };
    SLDataSink audioSnk = { &loc_outmix, NULL };
    SLObjectItf PlayerObject;
    // create audio player
    result = ( *m_engineEngine )->CreateAudioPlayer( m_engineEngine, &PlayerObject, &audioSrc, &audioSnk,
                                                   0, nullptr, nullptr );
    if ( PlayerObject != NULL )
    {
        ( *PlayerObject )->Destroy( PlayerObject );
        PlayerObject = NULL;
    }
    return result != SL_RESULT_SUCCESS;
}

bool OpenSLDevice::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( IsRecordingFormatSupported( nSampleRate, nChannels ) )
    {
        m_recSampleRate = nSampleRate;
        m_recChannel = nChannels;
        return true;
    }
    return false;
}

bool OpenSLDevice::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( IsPlayoutFormatSupported(nSampleRate,nChannels) )
    {
        m_plySampleRate = nSampleRate;
        m_plyChannel = nChannels;
        return true;
    }
    return false;
}

bool OpenSLDevice::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    nSampleRate = m_recSampleRate;
    nChannels = m_recChannel;
    return true;
}

bool OpenSLDevice::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    nSampleRate = m_plySampleRate;
    nChannels = m_plyChannel;
    return true;
}

bool OpenSLDevice::InitPlayout( )
{
    LOGW(" OpenSLDevice::InitPlayout enter");
    if ( !m_bInitialize )
    {
        return false;
    }

    if ( m_bInitPlayout )
    {
        return true;
    }

    m_outBufSamples = m_plySampleRate / 100 * m_plyChannel;
    for (int i = 0; i < kNumBuffer; i++)
    {
        m_outputBuffer[i] = (SLint16*)calloc( m_outBufSamples, sizeof( SLint16 ) );
    }

    for ( int i = 0; i < 100; i++ )
    {
        m_BufferPool.push_back( (SLint16*)calloc( m_outBufSamples, sizeof( SLint16 ) ));
    }

    if ( m_plyChannel != 1 && m_plyChannel != 2 )
    {
        return false;
    }


    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, kNumBuffer };
    const SLInterfaceID ids[] = { SL_IID_VOLUME };
    const SLboolean req[] = { SL_BOOLEAN_FALSE };
    result = ( *m_engineEngine )->CreateOutputMix( m_engineEngine, &m_outputMixObject, 1, ids, req );
    if ( result != SL_RESULT_SUCCESS )
        return result;

    // realize the output mix
    result = ( *m_outputMixObject )->Realize( m_outputMixObject, SL_BOOLEAN_FALSE );

    SLuint32 speakers;
    speakers = m_plyChannel > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;

    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, m_plyChannel, m_plySampleRate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        speakers, SL_BYTEORDER_LITTLEENDIAN };

    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, m_outputMixObject };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    // create audio player
    const SLInterfaceID ids1[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE,SL_IID_ANDROIDCONFIGURATION,SL_IID_ANDROIDEFFECT };
    const SLboolean req1[] = { SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE };
    result = ( *m_engineEngine )->CreateAudioPlayer( m_engineEngine, &m_plyObject, &audioSrc, &audioSnk,
                                                    2, ids1, req1 );
    if ( result != SL_RESULT_SUCCESS )
        return result;
    
    SLAndroidConfigurationItf playconfig;
    result = ( *m_plyObject )->GetInterface( m_plyObject, SL_IID_ANDROIDCONFIGURATION, &playconfig );
    if ( SL_RESULT_SUCCESS == result )
    {
        SLint32 streamType = SL_ANDROID_STREAM_VOICE;
        result = ( *playconfig )->SetConfiguration( playconfig,
                                                    SL_ANDROID_KEY_STREAM_TYPE, &streamType, sizeof( SLint32 ) );
    }


    result = ( *m_engineObject )->GetInterface( m_engineObject, SL_IID_ANDROIDEFFECTCAPABILITIES, &m_effectCapbilitiesobj );
    if ( result != SL_RESULT_SUCCESS )
    {
        LOGW( "the device is not supported the audio effect capbility:%ul", result );
    }
    else
    {
        SLInterfaceID effectType, effectImplementation;
        const SLuint16 FX_NAME_LENGTH = 256;
        SLuint16 effectNameLength = FX_NAME_LENGTH;
        SLchar effectName[FX_NAME_LENGTH + 1] = { 0 };
        SLuint32 nbEffects = 0;
        ( *m_effectCapbilitiesobj )->QueryNumEffects( m_effectCapbilitiesobj, &nbEffects );
        for ( SLuint32 i = 0; i < nbEffects; i++ )
        {
            memset( effectName, 0, FX_NAME_LENGTH + 1 );
            effectNameLength = FX_NAME_LENGTH;
            ( *m_effectCapbilitiesobj )->QueryEffect( m_effectCapbilitiesobj, i, &effectType, &effectImplementation, effectName, &effectNameLength );
            LOGW( "[audio_effect_capbilities]effect name:%s", (char*)effectName );
        }
    }




    // realize the player
    result = ( *m_plyObject )->Realize( m_plyObject, SL_BOOLEAN_FALSE );
    if ( result != SL_RESULT_SUCCESS )
        return result;

    // get the play interface
    result = ( *m_plyObject )->GetInterface( m_plyObject, SL_IID_PLAY, &m_plyPlayer );
    if ( result != SL_RESULT_SUCCESS )
        return result;

    // get the buffer queue interface
    result = ( *m_plyObject )->GetInterface( m_plyObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                &m_plyBufferQueue );
    if ( result != SL_RESULT_SUCCESS )
        return result;

    // register callback on the buffer queue
    result = ( *m_plyBufferQueue )->RegisterCallback( m_plyBufferQueue, PlayBufferQueueCallback, this );
    if ( result != SL_RESULT_SUCCESS )
        return result;



    m_bInitPlayout = true;
    LOGW( " OpenSLDevice::InitPlayout leave" );
    return true;
}


bool OpenSLDevice::InitRecording( )
{
    LOGW( "OpenSLDevice::InitRecording enter" );
    if ( !m_bInitialize )
    {
        return false;
    }

    if (m_bInitRecording)
    {
        return true;
    }

    m_inBufSamples = m_recSampleRate / 100 * m_recChannel;
    for ( int i = 0; i < kNumBuffer; i++ )
    {
        m_inputBuffer[i] = (SLint16*)calloc( m_inBufSamples, sizeof( SLint16 ) );
    }


    SLresult result;
    // configure audio source
    SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };

    SLDataSource audioSrc = { &loc_dev, NULL };

    // configure audio sink
    SLuint32 speakers = m_recChannel > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;

    SLDataLocator_AndroidSimpleBufferQueue loc_bq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, kNumBuffer };
    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, m_recChannel, m_recSampleRate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        speakers, SL_BYTEORDER_LITTLEENDIAN };
    SLDataSink audioSnk = { &loc_bq, &format_pcm };

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE,SL_IID_ANDROIDCONFIGURATION };
    const SLboolean req[] = { SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
    result = ( *m_engineEngine )->CreateAudioRecorder( m_engineEngine, &m_recObject, &audioSrc,
                                                        &audioSnk, 2, id, req );
    if ( SL_RESULT_SUCCESS != result ) return false;


    SLAndroidConfigurationItf recordconfig;
    result = ( *m_recObject )->GetInterface( m_recObject, SL_IID_ANDROIDCONFIGURATION, &recordconfig );
    if ( SL_RESULT_SUCCESS == result )
    {
        SLint32 streamType = SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION;
        result = ( *recordconfig )->SetConfiguration( recordconfig,
                                                      SL_ANDROID_KEY_RECORDING_PRESET, &streamType, sizeof( SLint32 ) );
        SLuint32 size = sizeof( SLint32 );
        result = ( *recordconfig )->GetConfiguration( recordconfig, SL_ANDROID_KEY_RECORDING_PRESET, &size, &streamType );
        if ( result != SL_RESULT_SUCCESS )
        {
            LOGW( "unsupport feature!!!" );
        }
        else
        {
            LOGW( "[OpenSLDevice] record GetConfiguration is:%d", streamType );
        }
    }

    // realize the audio recorder
    result = ( *m_recObject )->Realize( m_recObject, SL_BOOLEAN_FALSE );
    if ( SL_RESULT_SUCCESS != result ) return false;



    // get the record interface
    result = ( *m_recObject )->GetInterface( m_recObject, SL_IID_RECORD, &m_recRecorder );
    if ( SL_RESULT_SUCCESS != result ) return false;

    // get the buffer queue interface
    result = ( *m_recObject )->GetInterface( m_recObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                   &m_recBufferQueue );
    if ( SL_RESULT_SUCCESS != result ) return false;

    // register callback on the buffer queue
    result = ( *m_recBufferQueue )->RegisterCallback( m_recBufferQueue, RecordBufferQueueCallback,
                                                            this );
    if ( SL_RESULT_SUCCESS != result ) return false;


    m_bInitRecording = true;
    LOGW( "OpenSLDevice::InitRecording leave" );
    return true;
}


bool OpenSLDevice::StartPlayout()
{
    LOGW("OpenSLDevice::StartPlayOut() enter");
    if (!m_bInitialize || !m_bInitPlayout )
    {
        return false;
    }

    if (m_bPlaying)
    {
        return true;
    }

    // set the player's state to playing
    SLresult result = ( *m_plyPlayer )->SetPlayState( m_plyPlayer, SL_PLAYSTATE_PLAYING );
    if (result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    m_bPlaying = true;
    for ( int i = 0; i < kNumBuffer; ++i )
    {
        //EnqueuePlayoutData();
        // Enqueue the decoded audio buffer for playback.
        SLint16* buffer = m_outputBuffer[m_currentOutputBuffer];
        SLresult err = ( *m_plyBufferQueue )->Enqueue( m_plyBufferQueue, buffer, m_outBufSamples * 2 );
        if ( SL_RESULT_SUCCESS != err )
        {
            LOGW( "add playout data fail,err=%d", err );
        }
        m_currentOutputBuffer = ( m_currentOutputBuffer + 1 ) % kNumBuffer;
    }
    LOGW( "OpenSLDevice::StartPlayOut() leave" );

    return true;
}

bool OpenSLDevice::StopPlayout()
{
    if ( !m_bInitialize || !m_bInitPlayout )
    {
        return false;
    }

    if ( !m_bPlaying )
    {
        return true;
    }
    // set the player's state to playing
    SLresult result = ( *m_plyPlayer )->SetPlayState( m_plyPlayer, SL_PLAYSTATE_PAUSED ); // 暂停还是停止呢？
    if ( result != SL_RESULT_SUCCESS )
    {
        return false;
    }
    m_bPlaying = false;
    return true;
}

bool OpenSLDevice::Playing() const
{
    return m_bPlaying;
}

bool OpenSLDevice::StartRecording()
{
    LOGW( "OpenSLDevice::StartRecording() enter" );
    if (!m_bInitialize || !m_bInitRecording)
    {
        return false;
    }

    if (m_bRecording)
    {
        return true;
    }
    SLresult result = ( *m_recRecorder )->SetRecordState( m_recRecorder, SL_RECORDSTATE_RECORDING );
    if (result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    m_bRecording = true;
    for ( int i = 0; i < kNumBuffer; ++i )
    {
        SLint16* buffer = m_inputBuffer[m_currentInputBuffer];
        SLresult err = ( *m_recBufferQueue )->Enqueue( m_recBufferQueue, buffer, m_inBufSamples * sizeof( SLint16 ) );
        if ( SL_RESULT_SUCCESS != err )
        {
            LOGW( "get recording data fail,err=%d", err );
        }
        m_currentInputBuffer = ( m_currentInputBuffer + 1 ) % kNumBuffer;
    }
    LOGW( "OpenSLDevice::StartRecording() leave" );
    return true;
}

bool OpenSLDevice::StopRecording()
{
    if ( !m_bInitialize || !m_bInitPlayout )
    {
        return false;
    }

    if (!m_bRecording)
    {
        return true;
    }

    SLresult result = ( *m_recRecorder )->SetRecordState( m_recRecorder, SL_RECORDSTATE_PAUSED );
    if ( result != SL_RESULT_SUCCESS )
    {
        return false;
    }

    m_bRecording = false;

    return true;
}



bool OpenSLDevice::Recording() const
{
    return m_bRecording;
}

void OpenSLDevice::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    m_pBufferProc = pCallback;
}

bool OpenSLDevice::SetPropertie( AudioPropertyID id, void* )
{
    return false;
}

bool OpenSLDevice::GetProperty( AudioPropertyID id, void* )
{
    return false;
}

// this callback handler is called every time a buffer finishes playing
void OpenSLDevice::PlayBufferQueueCallback( SLAndroidSimpleBufferQueueItf bq, void *context )
{
   // LOGW(" OpenSLDevice::PlayBufferQueueCallback");
    OpenSLDevice *p = (OpenSLDevice*)context;
    p->FillBufferQueue();
}


// this callback handler is called every time a buffer finishes recording
void OpenSLDevice::RecordBufferQueueCallback( SLAndroidSimpleBufferQueueItf bq, void *context )
{
   // LOGW( " OpenSLDevice::RecordBufferQueueCallback" );
    OpenSLDevice *p = (OpenSLDevice *)context;
    p->EnqueueRecordData();
}

void OpenSLDevice::FillBufferQueue()
{
    if ( !m_bPlaying )
        return;
    EnqueuePlayoutData();
}

void OpenSLDevice::EnqueuePlayoutData()
{
    // Read audio data  from callback function registered by user to adjust for differences in buffer size between user and native
    // OpenSL ES.
    SLint16* buffer = m_outputBuffer[m_currentOutputBuffer];
   // Get play data from callback;
    int len = m_outBufSamples * 2;
    if ( m_pBufferProc )
    {
        len = m_pBufferProc->NeedMorePlayoutData( buffer, m_outBufSamples*sizeof( SLint16 ) );
    }
    else
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if ( !m_BufferList.empty() )
        {
            SLint16* data = m_BufferList.front();
            memcpy( buffer, data, m_outBufSamples * 2 );
            m_BufferList.pop_front();
            m_BufferPool.push_back( data );
        }
    }
    // Enqueue the decoded audio buffer for playback.
    SLresult err = ( *m_plyBufferQueue )->Enqueue( m_plyBufferQueue, buffer, m_outBufSamples * 2 );
    if ( SL_RESULT_SUCCESS != err )
    {
        LOGW( "add playout data fail,err=%d", err );
    }

    m_currentOutputBuffer = ( m_currentOutputBuffer + 1 ) % kNumBuffer;
}



void OpenSLDevice::EnqueueRecordData()
{
    if (!m_bRecording)
    {
        return;
    }
    SLint16* buffer = m_inputBuffer[m_currentInputBuffer];
    SLresult err = ( *m_recBufferQueue )->Enqueue( m_recBufferQueue, buffer, m_inBufSamples*sizeof(SLint16) );
    if ( SL_RESULT_SUCCESS != err )
    {
        LOGW( "get recording data fail,err=%d", err );
    }
    if ( m_pBufferProc )
    {
        m_pBufferProc->RecordingDataIsAvailable( buffer, m_inBufSamples * sizeof(SLint16) );
    }
    else
    {
        std::lock_guard<std::mutex> lg( m_lock );
        if (!m_BufferPool.empty())
        {
            SLint16*p = m_BufferPool.front();
            memcpy( p, buffer, m_inBufSamples * sizeof( SLint16 ) );
            m_BufferPool.pop_front();
            m_BufferList.push_back( p );
        }
    }
    m_currentInputBuffer = ( m_currentInputBuffer + 1 ) % kNumBuffer;

}