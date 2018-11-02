# In-Engine Audio and Video Capture Plugin for Unreal Engine 4

This plugin utilises the [MovieSceneCapture](https://api.unrealengine.com/INT/API/Runtime/MovieSceneCapture/index.html) and [AudioMixer](https://api.unrealengine.com/INT/API/Runtime/AudioMixer/index.html) modules to capture both audio and video from Unreal Engine 4 projects, without the need for external screen capture tools. This overcomes the lack of audio support when exporting movies using Sequencer, and also facilitates capturing audio and video when performing offscreen rendering on a headless device or inside an NVIDIA Docker container under Linux.

The plugin acts as a [MediaIPC](https://github.com/adamrehn/MediaIPC) producer process, transmitting data to any running MediaIPC consumer process that has been initialised with the prefix **"UE4Capture"**. For details on creating and using MediaIPC consumers, see the documentation from the [MediaIPC GitHub repository](https://github.com/adamrehn/MediaIPC).


## Contents

- [Requirements](#requirements)
- [Usage](#usage)
- [License](#license)


## Requirements

- Unreal Engine 4.19.0 or newer
- [CMake](https://cmake.org/) 3.8 or newer
- [ue4cli](https://github.com/adamrehn/ue4cli)
- [conan-ue4cli](https://github.com/adamrehn/conan-ue4cli)
- Under Unreal Engine 4.19, one of the public header files for the UE4 AudioMixer module includes a private header file, causing compilation errors when included by non-Engine modules. To fix this, run the script `patch-headers.py` in the [scripts](./scripts) directory. **This issue is fixed in Unreal Engine 4.20.**
- The Conan package for libMediaIPC needs to be compiled by running the command `build.py MediaIPC-ue4` from the [**ue4-conan-recipes**](https://github.com/adamrehn/ue4-conan-recipes) repository.
- Audio capture only works when using an output device based on the AudioMixer module, which requires running the game with the `-AudioMixer` command-line argument.


## Usage

Getting started with UE4Capture plugin is extremely simple. First, create a MediaIPC consumer to receive the capture output:

- Grab the [MediaIPC](https://github.com/adamrehn/MediaIPC) source code and build the CMake project, including the example producers and consumers.
- Run the executable for an example consumer (e.g [rawdump_consumer.cpp](https://github.com/adamrehn/MediaIPC/blob/master/examples/consumers/rawdump_consumer.cpp)) with the command-line argument **"UE4Capture"**, which will set the correct prefix value.
- The consumer should display the text *"Awaiting control block from producer process..."*.

Then, setup your UE4 project to act as the MediaIPC producer process:

- Create a new C++ UE4 project and copy the UE4Capture directory from the appropriate [plugin](./plugin) subdirectory for your Engine version to your project's `Plugins` directory.
- Modify your default game mode to inherit from the [ACaptureGameMode](./plugin/UE4Capture/Source/UE4Capture/Public/CaptureGameMode.h) base class *(may not work under Windows)*, or copy the code into your own game mode *(works under all platforms.)*
- Build the project by running the command `ue4 build` from the directory containing the project's .uproject file.
- Run the project by running the command `ue4 run -game -AudioMixer` from the directory containing the project's .uproject file.

The capture will now begin automatically:

- Once the game is running and the default level has loaded, switch back to the window for the consumer process. It should now display the text *"Receiving stream data from producer process..."*. This indicates that the capture has been initiated successfully.
- Once you close the window for the game, the consumer process will detect that the capture has ended and finalise its output, displaying the text *"Stream complete."*.
- In the case of the `rawdump_consumer` example, the output will be a pair of files (`audio.raw` and `video.raw`) containing the raw, uncompressed capture data. You will need to post-process these files using a tool such as [FFmpeg](https://www.ffmpeg.org/) or [GStreamer](https://gstreamer.freedesktop.org/) to encode the raw bitstreams into a usable video file.


## License

Copyright &copy; 2018, [Adam Rehn](https://github.com/adamrehn) and [Aidan Possemiers](https://github.com/ImmortalEmperor). Licensed under the MIT License, see the file [LICENSE](./LICENSE) for details.
