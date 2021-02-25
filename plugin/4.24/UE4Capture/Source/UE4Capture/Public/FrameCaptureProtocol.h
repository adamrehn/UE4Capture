#pragma once
//#include "VideoCapture.h"

#include <MediaIPC/MediaProducer.h>


#include "FrameGrabber.h"
#include "Protocols/FrameGrabberProtocol.h"

#include "CoreMinimal.h"
//#include "UObject/Object.h"

#include "MovieSceneCaptureSettings.h"

//#include "Misc/ScopeLock.h"
//#include "Misc/ScopeTryLock.h"
//#include "Templates/Casts.h"

#include "FrameCaptureProtocol.generated.h"

#ifndef LOCTEXT_NAMESPACE
#define LOCTEXT_NAMESPACE "FUE4CaptureModule"
#endif

//Our frame grabber settings, based on the defaults from FFrameGrabberProtocol
#define CAPTURE_PIXEL_FORMAT PF_B8G8R8A8
#define CAPTURE_NUM_SURFACES 3

//namespace
//{
	
struct CapturedFramePayload : public IFramePayload {
		FFrameMetrics Metrics;
};

UCLASS(meta = (DisplayName = "UE4Capture Frame Capture Protocol", CommandLineID = "UE4Capture"))
class UE4CAPTURE_API UFrameCaptureProtocol : public UFrameGrabberProtocol
{
public:
	GENERATED_BODY()

	UFrameCaptureProtocol(const FObjectInitializer& ObjInit) : Super(ObjInit)
	{
	}

	/*UFrameCaptureProtocol(const FObjectInitializer& ObjInit, TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer) {
		this->producer = producer;
	}*/

	void SetProducer(TSharedRef<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer) {
		this->producer = producer;
	}

	virtual bool SetupImpl() override
	{
		return Super::SetupImpl();
	}

	virtual void BeginFinalizeImpl() override
	{
		Super::BeginFinalizeImpl();
	}

	virtual void FinalizeImpl() override
	{
		Super::FinalizeImpl();
	}

	virtual bool HasFinishedProcessingImpl() const {
		return Super::HasFinishedProcessingImpl();
	}

	FFramePayloadPtr GetFramePayload(const FFrameMetrics& FrameMetrics)
	{
		TSharedRef<CapturedFramePayload, ESPMode::ThreadSafe> payload = MakeShareable(new CapturedFramePayload);
		payload->Metrics = FrameMetrics;
		return payload;
	}

	void ProcessFrame(FCapturedFrameData Frame)
	{
		//Attempt to lock our write lock
		//FScopeTryLock scopeLock(&this->writeLock);
		//if (scopeLock.IsLocked() == true)
		//{
			//Submit the captured framebuffer to our MediaIPC producer
		if (this->producer.IsValid()) {
			this->producer.Pin()->submitVideoFrame(Frame.ColorBuffer.GetData(), Frame.ColorBuffer.Num() * sizeof(FColor));
		}
		//}
	}

	virtual void CaptureFrameImpl(const FFrameMetrics& FrameMetrics) override
	{
		Super::CaptureFrameImpl(FrameMetrics);
	}

	void TickImpl() override
	{
		Super::TickImpl();
	}

private:

	TWeakPtr<MediaIPC::MediaProducer, ESPMode::ThreadSafe> producer;
	//TUniquePtr<FFrameGrabber> grabber;

	//UPROPERTY(config, EditAnywhere, Category = VideoSettings)
	//FCriticalSection writeLock;
};

