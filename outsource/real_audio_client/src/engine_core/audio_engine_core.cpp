#include "audio_engine_core.h"
#include "base\time_cvt.hpp"
namespace audio_engine{
	AudioEngineCore::AudioEngineCore()
	{

	}
	AudioEngineCore::~AudioEngineCore()
	{

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

	void AudioEngineCore::SetAudioDataCallback( IAudioDataCallback* cb )
	{
		_audio_data_cb_list.insert( cb );
	}

	void AudioEngineCore::RemoveAudioDataCallback( IAudioDataCallback* cb )
	{
		auto it = _audio_data_cb_list.find( cb );
		if (it != _audio_data_cb_list.end())
		{
			_audio_data_cb_list.erase( it );
		}
	}

	void AudioEngineCore::RecordingDataIsAvailable( const void* data, size_t size_in_byte )
	{
		auto buf = PreProcess( data, size_in_byte );
		
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
		buf->nsamples = size_in_byte/2;
		buf->channel = _rec_resampler.InChannel();
		buf->samplerate = _rec_resampler.InSamplerate();
		memcpy( buf->data, data, size_in_byte );
		buf->nsamples = size_in_byte / 2;
		for (auto it:_audio_data_cb_list)
		{
			it->CaptureData( buf );
		}
		auto outlen = _rec_resampler.Process((int16_t*)data, size_in_byte/2,buf->data,sizeof(buf->data)/2);
		assert( outlen != -1 );
		buf->nsamples = outlen;
		buf->channel = _rec_resampler.OutChannel();
		buf->samplerate = _rec_resampler.OutSamplerate();
		bool need_process = false;
		for(auto it : _audio_data_cb_list)
		{
			need_process = it->IsNeedProcess( );
		}
		if (need_process)
		{
			_audio_process.ProcessCaptureStream(buf);
			for (auto it: _audio_data_cb_list)
			{
				it->ProcessData( buf );
			}
		}
		return buf;
	}

}