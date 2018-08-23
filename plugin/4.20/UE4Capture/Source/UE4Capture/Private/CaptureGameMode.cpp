#include "CaptureGameMode.h"

#include "AVCapture.h"
#include "CaptureOptions.h"

ACaptureGameMode::ACaptureGameMode() : Super() {
	PrimaryActorTick.bCanEverTick = true;
}

void ACaptureGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Stop capturing
	if (this->capture.IsValid() == true)
	{
		this->capture->StopCapture(true);
		this->capture.Reset(nullptr);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ACaptureGameMode::Tick(float DeltaTime)
{
	//If this is our first Tick(), start capturing
	if (this->capture.IsValid() == false)
	{
		this->capture.Reset(new UE4Capture::AVCapture(UE4Capture::CaptureOptions::FromCommandLine()));
		this->capture->StartCapture();
	}
	
	Super::Tick(DeltaTime);
}
