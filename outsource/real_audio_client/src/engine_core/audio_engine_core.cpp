#include "audio_engine_core.h"
#include "base\time_cvt.hpp"
namespace audio_engine{
	AudioEngineCore::AudioEngineCore()
	{
		_encoder = _codec_factory.CreateEncoder( CODEC_OPUS, 0, 16000, 1,3000,20 );
	}

	AudioEngineCore::~AudioEngineCore()
	{
		if(_encoder)
			_encoder->Destroy();
	}

	void AudioEngineCore::Initialize()
	{
		_audio_device = AudioDevice::Create( AudioDevice::eCoreAudio );
		_audio_device->Initialize();
		uint32_t samplerate;
		uint16_t channel;
		if(_audio_device->GetRecordingFormat( samplerate, channel ))
		{
			_rec_resampler.Reset(samplerate,channel,16000,channel);
		}
	}

	void AudioEngineCore::Terminate()
	{
		if ( _audio_device )
		{
			_audio_device->Terminate();
			_audio_device->Release();
			_audio_device = nullptr;
		}
	}

	void AudioEngineCore::StartRecording()
	{
		_audio_device->StartRecording();
	}

	void AudioEngineCore::StartPlayout()
	{
		_audio_device->StartPlayout();
	}

	void AudioEngineCore::StopRecording()
	{
		_audio_device->StopRecording();
	}

	void AudioEngineCore::StopPlayout()
	{
		_audio_device->StopPlayout();
	}

	void AudioEngineCore::AddObserver( IAudioDataCallback* cb )
	{
		_cb_proxy.AddObserver( cb );
	}

	void AudioEngineCore::RemoveObserver( IAudioDataCallback* cb )
	{
		_cb_proxy.RemoveObserver( cb );
	}

	void AudioEngineCore::RecordingDataIsAvailable( const void* data, size_t size_in_byte )
	{
		AudioBufferPtr buf = PreProcess( data, size_in_byte );
		bool need_silent_filter = _cb_proxy.IsNeedSilentFilter();
		if ( need_silent_filter )
		{
		    buf = _silent_voice_filter.Process( buf );
		}
		_cb_proxy.BeforeEncodeData( buf );

		
		bool need_default_encoder = _cb_proxy.IsNeedDefaultEncode();

		if( need_default_encoder )
		{
			int outlen = _encoder->Process(buf->data,buf->length, buf->data, buf->length );
			if (outlen == 0)
			{
				return;
			}
			if (outlen < 0)
			{
				assert( 0 && "±àÂë³ö´í" );
				return;
			}
			buf->length = outlen;
			_cb_proxy.PostEncodeData( buf );
		}
	}

	size_t AudioEngineCore::NeedMorePlayoutData( void* data, size_t size_in_byte )
	{
		return 0;
	}

	void AudioEngineCore::ErrorOccurred( AudioError /*aeCode*/ )
	{

	}

	AudioBufferPtr AudioEngineCore::PreProcess( const void* data, size_t size_in_byte )
	{
		auto buf = std::make_shared<AudioBuffer>();
		buf->id = 0;
		buf->ts = TimeStamp();
		buf->length = size_in_byte;
		buf->channel = _rec_resampler.InChannel();
		buf->samplerate = _rec_resampler.InSamplerate();
		memcpy( buf->data, data, size_in_byte );
		_cb_proxy.CaptureData( buf );
		auto outlen = _rec_resampler.Process((int16_t*)data, size_in_byte/2,(int16_t*)buf->data,sizeof(buf->data)/2);
		assert( outlen != -1 );
		buf->length = outlen*2;
		buf->channel = _rec_resampler.OutChannel();
		buf->samplerate = _rec_resampler.OutSamplerate();
		bool need_process = _cb_proxy.IsNeedProcess();
		if (need_process)
		{
			_audio_process.ProcessCaptureStream(buf);
			_cb_proxy.ProcessData( buf );
		}
		return buf;
	}


	void AudioEngineCore::AudioDataCallbackProxy::CaptureData( AudioBufferPtr buffer )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		for (auto cb: _audio_data_cb_list)
		{
			cb->CaptureData( buffer );
		}
	}



	bool AudioEngineCore::AudioDataCallbackProxy::IsNeedProcess()
	{
		std::lock_guard<std::mutex> lock( _mutex );
		bool need_process = false;
		for( auto it : _audio_data_cb_list )
		{
			need_process = it->IsNeedProcess();
		}
		return need_process;
	}


	void AudioEngineCore::AudioDataCallbackProxy::ProcessData( AudioBufferPtr buf )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		for( auto it : _audio_data_cb_list )
		{
			it->ProcessData( buf );
		}
	}


	bool AudioEngineCore::AudioDataCallbackProxy::IsNeedSilentFilter()
	{
		std::lock_guard<std::mutex> lock( _mutex );
		bool need_silent_filter = true;
		for( auto it : _audio_data_cb_list )
		{
			if( !it->IsNeedSilentFilter() )
			{
				need_silent_filter = false;
			}
		}
		return need_silent_filter;
	}


	bool AudioEngineCore::AudioDataCallbackProxy::IsNeedDefaultEncode()
	{
		std::lock_guard<std::mutex> lock( _mutex );
		bool need_default_encoder = false;
		for( auto it : _audio_data_cb_list )
		{
			if( it->IsNeedDefaultEncode() )
			{
				need_default_encoder = true;
				break;
			}
		}
		return need_default_encoder;
	}


	void AudioEngineCore::AudioDataCallbackProxy::BeforeEncodeData( AudioBufferPtr buf )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		for( auto it : _audio_data_cb_list )
		{
			it->BeforeEncodeData( buf );
		}
	}

	
	void AudioEngineCore::AudioDataCallbackProxy::PostEncodeData( AudioBufferPtr buf )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		for( auto it : _audio_data_cb_list )
		{
			it->PostEncodeData( buf );
		}
	}

	bool AudioEngineCore::AudioDataCallbackProxy::DecodeData( AudioBufferPtr buffer )
	{
		return false;
	}


	void AudioEngineCore::AudioDataCallbackProxy::AfterDecodeData( AudioBufferPtr buffer )
	{

	}


	void AudioEngineCore::AudioDataCallbackProxy::MixerData( AudioBufferPtr buffer )
	{

	}


	void AudioEngineCore::AudioDataCallbackProxy::PlayoutData( AudioBufferPtr buffer )
	{

	}


	void AudioEngineCore::AudioDataCallbackProxy::AddObserver( IAudioDataCallback* cb )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		_audio_data_cb_list.insert( cb );
	}


	void AudioEngineCore::AudioDataCallbackProxy::RemoveObserver( IAudioDataCallback* cb )
	{
		std::lock_guard<std::mutex> lock( _mutex );
		auto it = _audio_data_cb_list.find( cb );
		if( it != _audio_data_cb_list.end() )
		{
			_audio_data_cb_list.erase( it );
		}
	}

}