#pragma once

#include <MediaIPC/MediaProducer.h>

#include "CaptureOptions.h"

namespace UE4Capture {

class AudioCapture;
class VideoCapture;

class AVCapture
{
	public:
		AVCapture(const CaptureOptions& options);
		~AVCapture();
		
		//These must only be called after the scene has finished loading
		void StartCapture();
		void StopCapture(bool isShuttingDown);
		
	private:
		TSharedPtr<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer;
		TUniquePtr<AudioCapture> audioCapture;
		TUniquePtr<VideoCapture> videoCapture;
		MediaIPC::ControlBlock cb;
		bool capturing;
};

} //End namespace UE4Capture
