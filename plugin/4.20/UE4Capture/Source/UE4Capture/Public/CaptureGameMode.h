#pragma once

#include "AVCapture.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaptureGameMode.generated.h"

UCLASS(minimalapi)
class ACaptureGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	public:
		ACaptureGameMode();
		
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		virtual void Tick(float DeltaTime) override;
		
	protected:
		TUniquePtr<UE4Capture::AVCapture> capture;
};
