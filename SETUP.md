# 🛠️ Setup – How to compile

If you want to have the [NodeCreator](https://github.com/InACTually/InACTually-NodeCreator) with the Engine, you need to clone `--recursive`, as it is a submodule.

To make it compile... pray. A CMake workflow would be a dream.
At the moment, only a project file for VisualStudio2022 is available, that are compatible with VisualStudio2026 (upgrade project files to v145).

Most of the dependencies are included in this repo (see the third-party notice below), but you need to setup the bigger (and optional) dependencies yourself:

## Needed Dependencies

+ [Cinder - 0.9.4dev](https://github.com/cinder/Cinder) [Jan'26] (or possibly newer) \
Place Cinder in a folder named "Cinder" parallel to your InACTually-Engine folder (same hierarchy level), so that it can be relatively found. You need to replace the ImGui files with its [docking-branch](https://github.com/ocornut/imgui/tree/docking) [Jan'26]. After that, your want to compile Cinder with dynamic runtime linking (in VisualStudio something like: Proj.-settings -> C/C++ -> CodeGeneration -> runtime lib -> "-DLL"):
    - for Release "Multithreaded-DLL (/MD)" name the output folder "Release_MD\..\" (e.g. name the configuration accordingly)
    - for Debug "Multithreaded-Debug-DLL (/MDd)" name the output folder "Debug_MD\..\" likewise

+ [OpenCV 4.10.0](https://github.com/opencv/opencv) (or possibly newer) \
You need to compile OpenCV, maybe with opencv_contrib and things you like to have.\
Place OpenCV in `./3rd/OpenCV4` – care: the include folder is `./3rd/OpenCV4/include/opencv`.\
You need to place/copy the corresponding *.dll's in `vc2022\bin\Release|Debug`.

## Optional ones

+ [ASIOSDK 2.3](https://www.steinberg.net/developers/asiosdk-open/) (or possibly newer) \
For optional multi-channel audio support, place the ASIOSDK in `./3rd/Cinder-PortAudio/lib/ASIOSDK` – care about the proprietary license. \
Standard build configurations are compiling without optional ASIOSDK, for including/using the ASIOSDK specific build targets are available.