#include "AudioCapture.h"
#include "Misc/CommandLine.h"
#include <memory>
#include <queue>

//Unique identifier for our audio submix effect
//(Chosen because UE 4.17.0 introduced the audio mixer system)
#define AUDIO_CAPTURE_SUBMIX_ID 4170

namespace
{
	class AudioCaptureSubmix : public FSoundEffectSubmix
	{
		public:
			
			AudioCaptureSubmix(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer, uint32 queueLength)
			{
				this->bIsActive = true;
				this->producer = producer;
				this->queueLength = queueLength;
			}
			
			virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData)
			{
				//Forward the input data to the output buffer
				OutData.AudioBuffer->Reset();
				OutData.AudioBuffer->Append(*InData.AudioBuffer);
				
				//Add the buffer of samples to our queue
				std::unique_ptr<float[]> buffer(new float[InData.AudioBuffer->Num()]);
				std::memcpy(buffer.get(), InData.AudioBuffer->GetData(), sizeof(float) * InData.AudioBuffer->Num());
				this->bufferQueue.emplace(std::move(buffer));
				
				//Flush any buffers that are ready to be submitted to our MediaIPC producer
				while (this->bufferQueue.size() > 2)
				{
					//Retrieve the next buffer
					std::unique_ptr<float[]> submissionBuffer = std::move(this->bufferQueue.front());
					this->bufferQueue.pop();
					
					//Submit it to our producer
					if (this->producer.IsValid() == true) {
						producer.Pin()->submitAudioSamples(submissionBuffer.get(), sizeof(float) * InData.AudioBuffer->Num());
					}
				}
			}
			
		private:
			TWeakPtr<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer;
			std::queue<std::unique_ptr<float[]>> bufferQueue;
			uint32 queueLength;
	};
}

namespace UE4Capture {

AudioCapture::AudioCapture()
{
	//Retrieve the active audio device
	this->mixer = (Audio::FMixerDevice*)(GEngine->GetActiveAudioDevice());
	this->capturing = false;
}

AudioCapture::~AudioCapture() {
	this->StopCapture();
}

void AudioCapture::PopulateControlBlock(MediaIPC::ControlBlock& cb)
{
	//Determine if we have a mixer-based audio device to capture from
	if (this->CanCapture() == true)
	{
		//Retrieve the audio parameters from the mixer
		FAudioPlatformSettings settings = this->mixer->GetPlatformSettings();
		cb.audioFormat = MediaIPC::AudioFormat::PCM_F32LE;
		cb.channels = this->mixer->GetNumDeviceChannels();
		cb.sampleRate = settings.SampleRate;
		cb.samplesPerBuffer = settings.CallbackBufferFrameSize;
	}
	else
	{
		//No mixer to capture from, disable audio capture
		cb.audioFormat = MediaIPC::AudioFormat::None;
		cb.channels = 0;
		cb.sampleRate = 0;
		cb.samplesPerBuffer = 0;
	}
}

void AudioCapture::StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer)
{
	//If we are already performing a capture, stop the previous capture
	this->StopCapture();
	
	//Don't attempt to perform a capture if we have no mixer device to capture from
	if (this->CanCapture() == false) {
		return;
	}
	
	//Retrieve the audio mixer settings so we can determine how many buffers to queue
	FAudioPlatformSettings settings = this->mixer->GetPlatformSettings();
	
	//Add our capture submix to the master submix
	AudioCaptureSubmix* captureSubmix = new AudioCaptureSubmix(producer, settings.NumBuffers);
	this->mixer->AddMasterSubmixEffect(AUDIO_CAPTURE_SUBMIX_ID, captureSubmix);
	this->capturing = true;
}

void AudioCapture::StopCapture()
{
	if (this->capturing == true)
	{
		//Remove our previously created capture submix
		this->mixer->RemoveMasterSubmixEffect(AUDIO_CAPTURE_SUBMIX_ID);
		this->capturing = false;
	}
}

bool AudioCapture::CanCapture()
{
	//Determine if we have a mixer-based audio device to capture from
	return (FParse::Param(FCommandLine::Get(), TEXT("AudioMixer")) && this->mixer != nullptr);
}

} //End namespace UE4Capture
