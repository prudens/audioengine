#include "java_wrapper_device.h"

JavaVM*   g_JavaVM = nullptr;
JNIEnv*   g_env = nullptr;
jclass    g_cls = nullptr;

void SetJavaVM( void* javaVM, void* context )
{
    if ( javaVM == nullptr )
    {
        return;
    }
    g_JavaVM = (JavaVM*)javaVM;
    g_JavaVM->GetEnv( (void**)&g_env, JNI_VERSION_1_6 );

    g_cls = g_env->FindClass( "com/audioengine/audio/AudioDevice.java" );

    g_cls = (jclass)g_env->NewGlobalRef( g_cls );//永久保存cls   
}

JavaWrapperDevice::JavaWrapperDevice()
{
	m_env = nullptr; 
	g_JavaVM->AttachCurrentThread((JNIEnv**)&m_env, NULL); 
    if (!m_env)
    {
		return;
    }
 
	auto idinit = m_env->GetMethodID( g_cls, "<init>", "()V" );
	m_audio_device_ins = m_env->NewObject( g_cls, idinit );  
}


JavaWrapperDevice::~JavaWrapperDevice()
{ 
    m_env->DeleteLocalRef( m_audio_device_ins );
    g_JavaVM->DetachCurrentThread();
}

void JavaWrapperDevice::Release()
{
    Terminate();
    delete this;
}

bool JavaWrapperDevice::Initialize()
{
    if (m_bInit)
    {
        return true;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "Attach", "(J)V" );
    if ( 0 == idmthod )
    {
        printf( "JavaWrapperDevice::Initialize get method Attach failed" );
        return false;
    }
    m_env->CallVoidMethod( m_audio_device_ins, idmthod, this );
    m_bInit = true;
    return true;
}

void JavaWrapperDevice::Terminate()
{
    if (!m_bInit)
    {
        return;
    }
    StopPlayout();
    StopRecording();
    m_bInit = false;
}

size_t JavaWrapperDevice::GetRecordingDeviceNum() const
{
    return 1;
}

size_t JavaWrapperDevice::GetPlayoutDeviceNum() const
{
    return 1;
}

bool JavaWrapperDevice::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}

bool JavaWrapperDevice::RecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}

bool JavaWrapperDevice::SetPlayoutDevice( int16_t index )
{
    return false;
}

bool JavaWrapperDevice::SetRecordingDevice( int16_t index )
{
    return false;
}

bool JavaWrapperDevice::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    if (!m_bInit)
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "IsRecordingFormatSupported", "(II)Z" );
    jboolean bSupported = m_env->CallBooleanMethod( m_audio_device_ins, idmthod, nSampleRate, nChannels );
    return bSupported != 0;
}

bool JavaWrapperDevice::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "IsPlayoutFormatSupported", "(II)Z" );
    jboolean bSupported = m_env->CallBooleanMethod( m_audio_device_ins, idmthod, nSampleRate, nChannels );
    return bSupported != 0;
}

bool JavaWrapperDevice::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "SetRecordingFormat", "(II)Z" );
    jboolean bSupported = m_env->CallBooleanMethod( m_audio_device_ins, idmthod, nSampleRate, nChannels );
    return bSupported != 0;
}

bool JavaWrapperDevice::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "SetPlayoutFormat", "(II)Z" );
    jboolean bSupported = m_env->CallBooleanMethod( m_audio_device_ins, idmthod, nSampleRate, nChannels );
    return bSupported != 0;
}

bool JavaWrapperDevice::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "GetRecordingFormat", "(V)I" );
    jint format = m_env->CallIntMethod( m_audio_device_ins, idmthod );
    nChannels = format >> 31 ? 2 : 1;
    nSampleRate = format & 0xffff; // 最多48000
    return true;
}

bool JavaWrapperDevice::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "GetPlayoutFormat", "(V)I" );
    jint format = m_env->CallIntMethod( m_audio_device_ins, idmthod );
    nChannels = format >> 31 ? 2 : 1;
    nSampleRate = format & 0xffff; // 最多48000
    return true;
}

bool JavaWrapperDevice::InitPlayout()
{
    if (!m_bInit)
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "InitPlayout", "(V)Z" );
    jboolean bInit = m_env->CallBooleanMethod( m_audio_device_ins ,idmthod);
    m_bInitPlayout = bInit;
    return m_bInitPlayout;
}

bool JavaWrapperDevice::InitRecording()
{
    if ( !m_bInit )
    {
        return false;
    }
    jmethodID idmthod = m_env->GetMethodID( g_cls, "InitRecording", "(V)Z" );
    jboolean bInit = m_env->CallBooleanMethod( m_audio_device_ins, idmthod );
    m_bInitRecording = bInit;
    return m_bInitRecording;
}

bool JavaWrapperDevice::StartPlayout()
{
    if (!m_bInit)
    {
        return false;
    }

    if (!m_bInitPlayout)
    {
        return false;
    }

    if (m_bPlaying)
    {
        return true;
    }

    jmethodID idmthod  = m_env->GetMethodID( g_cls, "StartPlayout", "(V)Z" );
    jboolean  bPlaying = m_env->CallBooleanMethod( m_audio_device_ins, idmthod );
    m_bPlaying = bPlaying;
    return m_bPlaying;
}

bool JavaWrapperDevice::StopPlayout()
{
    if (!m_bPlaying)
    {
        return true;
    }

    jmethodID idmthod = m_env->GetMethodID( g_cls, "StopPlayout", "(V)Z" );
    jboolean  bStop = m_env->CallBooleanMethod( m_audio_device_ins, idmthod );
    m_bPlaying = false;
    return bStop;
}

bool JavaWrapperDevice::Playing() const
{
    return m_bPlaying;
}

bool JavaWrapperDevice::StartRecording()
{
    if ( !m_bInit )
    {
        return false;
    }

    if ( !m_bInitRecording )
    {
        return false;
    }

    if ( m_bRecording )
    {
        return true;
    }

    jmethodID idmthod = m_env->GetMethodID( g_cls, "StartRecording", "(V)Z" );
    jboolean  bRecording = m_env->CallBooleanMethod( m_audio_device_ins, idmthod );
    m_bRecording = bRecording;
    return m_bRecording;
}

bool JavaWrapperDevice::StopRecording()
{
    if ( !m_bRecording )
    {
        return true;
    }

    jmethodID idmthod = m_env->GetMethodID( g_cls, "StopRecording", "(V)Z" );
    jboolean  bStop = m_env->CallBooleanMethod( m_audio_device_ins, idmthod );
    m_bRecording = false;
    return bStop;
}

bool JavaWrapperDevice::Recording() const
{
    return m_bRecording;
}

void JavaWrapperDevice::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    m_pBufferProc = pCallback;
}

bool JavaWrapperDevice::SetPropertie( AudioPropertyID id, void* )
{
    return false;
}

bool JavaWrapperDevice::GetProperty( AudioPropertyID id, void* )
{
    return false;
}


void JavaWrapperDevice::RecordDataIsAvailable( int32_t frame_size )
{
    if (m_pBufferProc)
    {
        m_pBufferProc->RecordingDataIsAvailable( m_rec_direct_buffer_address, frame_size );
    }
}

void JavaWrapperDevice::NeedMorePlayData( int32_t frame_size )
{
    if (m_pBufferProc)
    {
        m_pBufferProc->NeedMorePlayoutData( m_ply_direct_buffer_address, frame_size );
    }
}

void JavaWrapperDevice::CacheDirectBufferAddress( void* rec_direct_buffer_address, 
                                                    size_t rec_direct_buffer_capacity_in_bytes,
                                                    void* ply_direct_buffer_address, 
                                                    size_t ply_direct_buffer_capacity_in_bytes )
{
    m_rec_direct_buffer_address = rec_direct_buffer_address;
    m_rec_direct_buffer_capacity_in_bytes = rec_direct_buffer_capacity_in_bytes;
    m_ply_direct_buffer_address = ply_direct_buffer_address;
    m_ply_direct_buffer_capacity_in_bytes = ply_direct_buffer_capacity_in_bytes;
}

void JNICALL Java_com_audioengine_audio_AudioDevice_RecordDataIsAvailable
( JNIEnv *env, jobject, jlong ins, jint byte_size )
{
	JavaWrapperDevice* pDev = (JavaWrapperDevice*)ins;
	if(pDev) pDev->RecordDataIsAvailable( byte_size);
}

void JNICALL Java_com_audioengine_audio_AudioDevice_NeedMorePlayData
( JNIEnv *env, jobject, jlong ins, jint len )
{
	JavaWrapperDevice* pDev = (JavaWrapperDevice*)ins;
	if(pDev) pDev->NeedMorePlayData(  len );
}

JNIEXPORT void JNICALL Java_com_audioengine_audio_AudioDevice_CacheDirectBufferAddress
( JNIEnv *env, jobject, jlong ins, jobject rec_byte_buffer, jobject ply_byte_buffer )
{
    JavaWrapperDevice* pDev = (JavaWrapperDevice*)ins;
    void* rec_direct_buffer_address = env->GetDirectBufferAddress( rec_byte_buffer );
    jlong rec_capacity = env->GetDirectBufferCapacity( rec_byte_buffer );
    size_t rec_direct_buffer_capacity_in_bytes = static_cast<size_t>( rec_capacity );

    void* ply_direct_buffer_address = env->GetDirectBufferAddress( ply_byte_buffer );
    jlong ply_capacity = env->GetDirectBufferCapacity( ply_byte_buffer );
    size_t ply_direct_buffer_capacity_in_bytes = static_cast<size_t>( ply_capacity );

    pDev->CacheDirectBufferAddress( rec_direct_buffer_address, rec_direct_buffer_capacity_in_bytes, ply_direct_buffer_address, ply_direct_buffer_capacity_in_bytes );
}