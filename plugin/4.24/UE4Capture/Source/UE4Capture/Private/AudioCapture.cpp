#include "AudioCapture.h"
#include "AudioMixerDevice.h"
#include "Misc/CommandLine.h"
#include <memory>
#include <queue>

class AudioCaptureListener : public ISubmixBufferListener
{
	public:
		
		AudioCaptureListener(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer, uint32 queueLength)
		{
			this->producer = producer;
			this->queueLength = queueLength;
		}
		
		virtual ~AudioCaptureListener() {}
		
		virtual void OnNewSubmixBuffer(const USoundSubmix * OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock) 
		{
			//Add the buffer of samples to our queue
			std::unique_ptr<float[]> buffer(new float[NumSamples]);
			std::memcpy(buffer.get(), AudioData, sizeof(float) * NumSamples);
			this->bufferQueue.emplace(std::move(buffer));
			
			//Flush any buffers that are ready to be submitted to our MediaIPC producer
			while (this->bufferQueue.size() > 2)
			{
				//Retrieve the next buffer
				std::unique_ptr<float[]> submissionBuffer = std::move(this->bufferQueue.front());
				this->bufferQueue.pop();
				
				//Submit it to our producer
				if (this->producer.IsValid() == true) {
					producer.Pin()->submitAudioSamples(submissionBuffer.get(), sizeof(float) * NumSamples);
				}
			}
		}
		
	private:
		TWeakPtr<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer;
		std::queue<std::unique_ptr<float[]>> bufferQueue;
		uint32 queueLength;
};

namespace UE4Capture {

AudioCapture::AudioCapture()
{
	//Retrieve the active audio device
	this->mixer = GEngine->GetActiveAudioDevice();
	this->capturing = false;
}

AudioCapture::~AudioCapture() {
	this->StopCapture(false);
}

void AudioCapture::PopulateControlBlock(MediaIPC::ControlBlock& cb)
{
	//Determine if we have a mixer-based audio device to capture from
	if (this->CanCapture() == true)
	{
		//Retrieve the audio parameters from the mixer
		FAudioPlatformSettings settings = this->mixer->PlatformSettings;
		cb.audioFormat = MediaIPC::AudioFormat::PCM_F32LE;
		cb.channels = ((Audio::FMixerDevice*)this->mixer)->GetDeviceOutputChannels();
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
	this->StopCapture(false);
	
	//Don't attempt to perform a capture if we have no mixer device to capture from
	if (this->CanCapture() == false) {
		return;
	}
	
	//Retrieve the audio mixer settings so we can determine how many buffers to queue
	FAudioPlatformSettings settings = this->mixer->PlatformSettings;
	
	//Add our buffer listener to the master submix
	this->listener.Reset(new AudioCaptureListener(producer, settings.NumBuffers));
	this->mixer->RegisterSubmixBufferListener(this->listener.Get(), nullptr);
	this->capturing = true;
}

void AudioCapture::StopCapture(bool isShuttingDown)
{
	if (this->capturing == true)
	{
		//Attempting to call UnregisterSubmixBufferListener() during shutdown causes a segfault,
		//so only unregister our listener if we are not currently shutting the game down
		if (isShuttingDown == false) {
			this->mixer->UnregisterSubmixBufferListener(this->listener.Get(), nullptr);
		}
		
		this->capturing = false;
	}
}

bool AudioCapture::CanCapture()
{
	//Determine if we have a mixer-based audio device to capture from
	return (FParse::Param(FCommandLine::Get(), TEXT("AudioMixer")) && this->mixer != nullptr);
}

} //End namespace UE4Capture
