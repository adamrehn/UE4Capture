#include "AVCapture.h"
#include "UE4Capture.h"
#include "AudioCapture.h"
#include "VideoCapture.h"
#include "Engine.h"

namespace UE4Capture {

AVCapture::AVCapture(const CaptureOptions& options)
{
	//Create our control block
	this->cb = MediaIPC::ControlBlock();
	
	//Create our audio capture manager and populate the audio parameters of our control block
	this->audioCapture.Reset(new AudioCapture());
	this->audioCapture->PopulateControlBlock(this->cb);
	
	//Create our video capture manager and populate the video parameters of our control block
	this->videoCapture.Reset(new VideoCapture(options.framerate));
	this->videoCapture->PopulateControlBlock(this->cb);
	
	//Don't start capturing until StartCapture() is called
	this->capturing = false;
}

AVCapture::~AVCapture() {
	this->StopCapture(false);
}

void AVCapture::StartCapture()
{
	//If we are already performing a capture, stop the previous capture
	this->StopCapture(false);
	
	//Create our MediaIPC producer
	this->producer = MakeShared<MediaIPC::MediaProducer, ESPMode::ThreadSafe>("UE4Capture", this->cb);
	
	//Start audio capture (if enabled)
	if (this->audioCapture.IsValid()) {
		this->audioCapture->StartCapture(this->producer.ToSharedRef());
	}
	
	//Start video capture (if enabled)
	if (this->videoCapture.IsValid()) {
		this->videoCapture->StartCapture(this->producer.ToSharedRef());
	}
	
	this->capturing = true;
}

void AVCapture::StopCapture(bool isShuttingDown)
{
	if (this->capturing == true)
	{
		//Stop our MediaIPC producer
		this->producer->stop();
		this->producer.Reset();
		
		//Stop audio capture
		if (this->audioCapture.IsValid()) {
			this->audioCapture->StopCapture(isShuttingDown);
		}
		
		//Stop video capture
		if (this->videoCapture.IsValid()) {
			this->videoCapture->StopCapture(isShuttingDown);
		}
		
		this->capturing = false;
	}
}

} //End namespace UE4Capture
