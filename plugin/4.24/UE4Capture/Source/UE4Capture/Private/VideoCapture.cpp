#include "VideoCapture.h"

#include <MediaIPC/MediaProducer.h>

#include "FrameCaptureProtocol.h"

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "MovieSceneCapture.h"

#ifndef LOCTEXT_NAMESPACE
#define LOCTEXT_NAMESPACE "FUE4CaptureModule"
#endif

//Our frame grabber settings, based on the defaults from FFrameGrabberProtocol
#define CAPTURE_PIXEL_FORMAT PF_B8G8R8A8
#define CAPTURE_NUM_SURFACES 3

//Attempts to retrieve the scene viewport
TSharedPtr<FSceneViewport> getViewport()
{
	//Attempt to retrieve the scene viewport
	UGameEngine* engine = Cast<UGameEngine>(GEngine);
	if (engine != nullptr && engine->SceneViewport->GetClient()->GetWorld() != nullptr) {
		return engine->SceneViewport;
	}
	
	return nullptr;
}


namespace UE4Capture {

VideoCapture::VideoCapture(uint8 frameRate)
{
	//this->protocolID = TEXT("UE4Capture");
	this->frameRate = frameRate;
	this->capturing = false;
}

VideoCapture::~VideoCapture() {
	this->StopCapture(false);
}

void VideoCapture::PopulateControlBlock(MediaIPC::ControlBlock& cb)
{
	//Attempt to retrieve the scene viewport
	TSharedPtr<FSceneViewport> viewport = getViewport();
	if (viewport.IsValid() == true)
	{
		//Retrieve the resolution from the viewport
		FIntPoint resolution = viewport->GetSize();
		cb.videoFormat = MediaIPC::VideoFormat::BGRA;
		cb.width = resolution.X;
		cb.height = resolution.Y;
		cb.frameRate = this->frameRate;
	}
	else
	{
		//Could not retrieve the viewport, disable video capture
		cb.videoFormat = MediaIPC::VideoFormat::None;
		cb.width = 0;
		cb.height = 0;
		cb.frameRate = 0;
	}
}

void VideoCapture::StartCapture(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer)
{
	//If we are already performing a capture, stop the previous capture
	this->StopCapture(false);
	
	//Don't attempt to perform a capture if we have no viewport to capture from
	if (this->CanCapture() == false) {
		return;
	}

	//Our frame capture protocol details
	//FMovieSceneCaptureProtocolInfo protoInfo;
	//protoInfo.DisplayName = LOCTEXT("UE4Capture", "UE4Capture Frame Capture Protocol");
	//protoInfo.SettingsClassType = UMovieSceneCaptureSettings::StaticClass();
	//protoInfo.Factory = [this, producer]() -> TSharedRef<UFrameGrabberProtocol> {
	//	return MakeShareable(new FrameCaptureProtocol(producer));
	//};

	//UFrameCaptureProtocol* FCProto
	//TFunction<TSharedRef<UMovieSceneImageCaptureProtocolBase>()> Factory = [this, producer]() -> TSharedRef<UFrameCaptureProtocol> {
	//	return MakeShareable(new UFrameCaptureProtocol(producer));
	//};*/

	//Register our frame capture protocol
	//auto& protoRegistry = IMovieSceneCaptureModule::Get().GetProtocolRegistry();
	//protoRegistry.RegisterProtocol(this->protocolID, protoInfo);


	FString ProtocolName = TEXT("FrameCaptureProtocol");
	UFrameCaptureProtocol* FrameCaptureProto = NewObject<UFrameCaptureProtocol>();
	UClass* ImageProtocolType = FrameCaptureProto->GetClass();


	//Attempt to retrieve the scene viewport
	TSharedPtr<FSceneViewport> viewport = getViewport();
	if (viewport.IsValid() == true)
	{
		//Create our capture object
		capture = NewObject<UMovieSceneCapture>(GetTransientPackage());
		capture->Settings.FrameRate = FFrameRate(this->frameRate, 1);
		
		//capture->CaptureType = this->protocolID;
		capture->SetImageCaptureProtocolType(ImageProtocolType);

		capture->PostInitProperties();
		capture->Initialize(viewport.ToSharedRef());
		
		//Set the producer before start capture
		((UFrameCaptureProtocol*) capture->ImageCaptureProtocol)->SetProducer(producer);

		capture->StartCapture();
	}
	
	//Immediately un-register our frame capture protocol
	//protoRegistry.UnRegisterProtocol(this->protocolID);
	
	this->capturing = true;
}

void VideoCapture::StopCapture(bool isShuttingDown)
{
	if (this->capturing == true)
	{
		//Finalise and destroy our capture object
		//if (capture.IsValid())
		if (capture)
		{
			capture->Finalize();
			capture->Close();

			capture->ImageCaptureProtocol = nullptr;

			//capture.Reset();
			//capture->Destroy();
		}
		
		this->capturing = false;
	}
}

bool VideoCapture::CanCapture()
{
	//Determine if we can retrieve the scene viewport
	TSharedPtr<FSceneViewport> viewport = getViewport();
	return (viewport.IsValid() == true);
}

} //End namespace UE4Capture
