#include "VideoCapture.h"

#include <MediaIPC/MediaProducer.h>

#include "FrameGrabber.h"
#include "IMovieSceneCaptureProtocol.h"
#include "MovieSceneCapture.h"
#include "MovieSceneCaptureModule.h"
#include "MovieSceneCaptureProtocolSettings.h"
#include "Misc/ScopeLock.h"
#include "Misc/ScopeTryLock.h"
#include "Templates/Casts.h"

#ifndef LOCTEXT_NAMESPACE
#define LOCTEXT_NAMESPACE "FUE4CaptureModule"
#endif

//Our frame grabber settings, based on the defaults from FFrameGrabberProtocol
#define CAPTURE_PIXEL_FORMAT PF_B8G8R8A8
#define CAPTURE_NUM_SURFACES 3

namespace
{
	struct CapturedFramePayload : public IFramePayload {
		FFrameMetrics Metrics;
	};
	
	class FrameCaptureProtocol : public IMovieSceneCaptureProtocol
	{
		public:
			FrameCaptureProtocol(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer) {
				this->producer = producer;
			}
			
			virtual bool Initialize(const FCaptureProtocolInitSettings& InSettings, const ICaptureProtocolHost& Host) override
			{
				//Create our frame grabber
				this->grabber.Reset(new FFrameGrabber(InSettings.SceneViewport.ToSharedRef(), InSettings.DesiredSize, CAPTURE_PIXEL_FORMAT, CAPTURE_NUM_SURFACES));
				this->grabber->StartCapturingFrames();
				return true;
			}
			
			virtual void Finalize() override
			{
				//Ensure we wait until ProcessFrame() has exited the critical section before cleaning up
				FScopeLock scopeLock(&this->writeLock);
				this->grabber->StopCapturingFrames();
				this->grabber->Shutdown();
				this->grabber.Reset();
			}
			
			virtual bool HasFinishedProcessing() const {
				return (this->grabber->HasOutstandingFrames() == false);
			}
			
			FFramePayloadPtr GetFramePayload(const FFrameMetrics& FrameMetrics, const ICaptureProtocolHost& Host)
			{
				//Wrap the supplied metrics data in our custom payload class
				TSharedRef<CapturedFramePayload, ESPMode::ThreadSafe> payload = MakeShareable(new CapturedFramePayload);
				payload->Metrics = FrameMetrics;
				return payload;
			}
			
			void ProcessFrame(FCapturedFrameData Frame)
			{
				//Attempt to lock our write lock
				FScopeTryLock scopeLock(&this->writeLock);
				if (scopeLock.IsLocked() == true)
				{
					//Submit the captured framebuffer to our MediaIPC producer
					if (this->producer.IsValid()) {
						this->producer.Pin()->submitVideoFrame(Frame.ColorBuffer.GetData(), Frame.ColorBuffer.Num() * sizeof(FColor));
					}
				}
			}
			
			virtual void CaptureFrame(const FFrameMetrics& FrameMetrics, const ICaptureProtocolHost& Host) override {
				this->grabber->CaptureThisFrame(GetFramePayload(FrameMetrics, Host));
			}
			
			void Tick() override
			{
				//Process the most recent frame that has been captured since the previous Tick()
				TArray<FCapturedFrameData> frames = this->grabber->GetCapturedFrames();
				if (frames.Num() > 0) {
					this->ProcessFrame(MoveTemp(frames.Last()));
				}
			}
		
		private:
			TWeakPtr<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer;
			TUniquePtr<FFrameGrabber> grabber;
			FCriticalSection writeLock;
	};
	
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
}

namespace UE4Capture {

VideoCapture::VideoCapture(uint8 frameRate)
{
	this->protocolID = TEXT("UE4Capture");
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
	FMovieSceneCaptureProtocolInfo protoInfo;
	protoInfo.DisplayName = LOCTEXT("UE4Capture", "UE4Capture Frame Capture Protocol");
	protoInfo.SettingsClassType = UMovieSceneCaptureProtocolSettings::StaticClass();
	protoInfo.Factory = [this, producer]() -> TSharedRef<IMovieSceneCaptureProtocol> {
		return MakeShareable(new FrameCaptureProtocol(producer));
	};
	
	//Register our frame capture protocol
	auto& protoRegistry = IMovieSceneCaptureModule::Get().GetProtocolRegistry();
	protoRegistry.RegisterProtocol(this->protocolID, protoInfo);
	
	//Attempt to retrieve the scene viewport
	TSharedPtr<FSceneViewport> viewport = getViewport();
	if (viewport.IsValid() == true)
	{
		//Create our capture object
		capture.Reset(NewObject<UMovieSceneCapture>(GetTransientPackage()));
		capture->Settings.FrameRate = FFrameRate(this->frameRate, 1);
		capture->CaptureType = this->protocolID;
		capture->PostInitProperties();
		capture->Initialize(viewport.ToSharedRef());
		capture->StartCapture();
	}
	
	//Immediately un-register our frame capture protocol
	protoRegistry.UnRegisterProtocol(this->protocolID);
	
	this->capturing = true;
}

void VideoCapture::StopCapture(bool isShuttingDown)
{
	if (this->capturing == true)
	{
		//Finalise and destroy our capture object
		if (capture.IsValid())
		{
			capture->Finalize();
			capture->Close();
			capture.Reset();
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
