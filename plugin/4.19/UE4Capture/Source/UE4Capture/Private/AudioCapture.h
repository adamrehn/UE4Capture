#pragma once

#include <MediaIPC/MediaProducer.h>

#include "AudioMixerDevice.h"
#include "CoreMinimal.h"

namespace UE4Capture {

class AudioCapture
{
	public:
		AudioCapture();
		~AudioCapture();
		
		void PopulateControlBlock(MediaIPC::ControlBlock& cb);
		void StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer);
		void StopCapture();
		
	private:
		bool CanCapture();
		
		Audio::FMixerDevice* mixer;
		bool capturing;
};

} //End namespace UE4Capture
