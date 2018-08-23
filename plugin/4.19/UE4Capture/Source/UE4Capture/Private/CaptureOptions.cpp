#include "CaptureOptions.h"
#include "Misc/CommandLine.h"

namespace UE4Capture {

CaptureOptions CaptureOptions::FromCommandLine()
{
	//Fallback to our sensible defaults for any unspecified values
	CaptureOptions options;
	
	//Parse any specified framerate
	uint8 framerate = options.framerate;
	if (FParse::Value(FCommandLine::Get(), TEXT("-CaptureFramerate="), framerate) == true) {
		options.framerate = framerate;
	}
	
	return options;
}

} //End namespace UE4Capture
