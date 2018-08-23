#pragma once

#include <MediaIPC/MediaProducer.h>

#include "CoreMinimal.h"
#include "MovieSceneCaptureProtocolRegistry.h"
#include "MovieSceneCaptureProtocolSettings.h"

class UMovieSceneCapture;

namespace UE4Capture {

class VideoCapture
{
	public:
		VideoCapture(uint8 frameRate);
		~VideoCapture();
		
		void PopulateControlBlock(MediaIPC::ControlBlock& cb);
		void StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer);
		void StopCapture(bool isShuttingDown);
		
	private:
		bool CanCapture();
		
		TUniquePtr<UMovieSceneCapture> capture;
		FCaptureProtocolID protocolID;
		uint8 frameRate;
		bool capturing;
};

} //End namespace UE4Capture
