#pragma once

#include <MediaIPC/MediaProducer.h>

#include "AudioDevice.h"
#include "CoreMinimal.h"

class AudioCaptureListener;

namespace UE4Capture {

class AudioCapture
{
	public:
		AudioCapture();
		~AudioCapture();
		
		void PopulateControlBlock(MediaIPC::ControlBlock& cb);
		void StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer);
		void StopCapture(bool isShuttingDown);
		
	private:
		bool CanCapture();
		
		TUniquePtr<AudioCaptureListener> listener;
		FAudioDevice* mixer;
		bool capturing;
};

} //End namespace UE4Capture
