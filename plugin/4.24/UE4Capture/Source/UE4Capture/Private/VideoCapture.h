#pragma once

#include <MediaIPC/MediaProducer.h>

#include "CoreMinimal.h"

#include "MovieSceneCapture.h"


class UMovieSceneCapture;

namespace UE4Capture {

class UE4CAPTURE_API VideoCapture
{
	public:
		VideoCapture(uint8 frameRate);
		~VideoCapture();
		
		void PopulateControlBlock(MediaIPC::ControlBlock& cb);
		void StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer);
		void StopCapture(bool isShuttingDown);
		
	private:
		bool CanCapture();
		
		//TUniquePtr<UMovieSceneCapture> capture; // TUniquePtr oder TSHaredPtr cannot have UObjects applied. They will crash when destrucuted. (https://dawnarc.com/2018/07/ue4-tsharedptr-tweakobjectptr-and-tuniqueptr/)
		UMovieSceneCapture* capture;

		//FCaptureProtocolID protocolID;
		uint8 frameRate;
		bool capturing;
};

} //End namespace UE4Capture
