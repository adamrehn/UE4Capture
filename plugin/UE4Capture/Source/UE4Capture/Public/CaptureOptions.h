#pragma once

#include <string>
#include <stdint.h>

namespace UE4Capture {

class CaptureOptions
{
	public:
		
		//The framerate to capture video at
		uint8 framerate;
		
		//Constructs an options object using a set of sensible defaults
		CaptureOptions() : framerate(30) {}
		
		//Constructs an options object using the supplied values
		CaptureOptions(uint8 rate) : framerate(rate) {}
		
		//Constructs an options object by parsing the relevant command-line parameters
		static CaptureOptions FromCommandLine();
};

} //End namespace UE4Capture
