// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UE4Capture.h"

#define LOCTEXT_NAMESPACE "FUE4CaptureModule"

DEFINE_LOG_CATEGORY(LogUE4Capture);

void FUE4CaptureModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUE4CaptureModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUE4CaptureModule, UE4Capture)
