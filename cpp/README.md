# C++ Examples for IC Imaging Control 4

This directory contains a selection of example programs demonstrating the use of the IC Imaging Control 4 C++ API.

Most of the examples focus on a particular aspect of writing software to control a camera. Others, like [qt6/demoapp](qt6/demoapp) are small complete application programs.

## Download

To download all examples in a .zip file, move to the root of the repository, click the green *Code* button and select *Download ZIP*.

You can also select *Open with Visual Studio* to clone the examples into a directory on your computer, or use git directly.

After downloading, please make sure to keep the per-language directory structure intact so that relative file references can work.

***Warning:*** Be careful when cloning/extracting to a directory with a long path name; many tools are still limited by Windows' legacy 260-character path length limit and can raise obscure error messages in case a file path exceeds it.

# Prerequisites for Bulding and Testing the Example Programs

All examples assume you have access to an industrial camera from [The Imaging Source](www.theimagingsource.com).

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) matching your camera is installed.

All C++ examples assume that the IC4 SDK for C++ is installed. For Windows, this means installing *IC Imaging Control 4 SDK* from the [website](https://www.theimagingsource.com/en-us/support/download/). For Linux, all debian packages from *ic4.tar.gz* should be installed.

The examples require *CMake* (On *Windows*, this is provided by *Visual Studio 2019* or later) and a C++ compiler supporting at least C++14.

# Compiling the Example Programs

Most examples are built using *CMake*. You can either manually use *CMake* to build the example programs, or use an IDE that knows how to build *CMake* projects.

A group of example programs can be compiled by running *CMake* in one of the parent directories containing a `CMakeLists.txt` file.
Please note that programs depending on third-party libraries like *Qt* or *OpenCV* will not be built unless the respective libraries are found.

## Compiling the Example Programs Using Visual Studio

*Visual Studio* natively support *CMake*' projects from version 2019 onwards.

From the main menu, select *File -> Open... -> Folder...* and navigate to a directory containing a `CMakeLists.txt` file.

After the project was configured successfully, select the example executable you want to build and run from *Select Startup Item...* dropdown next to the *Start Debugging* button.

Click *Start Debugging* and the program will be compiled and run.

## Compiling the Example Programs Using cmake from the Command Line

First, enter a directory containing a `CMakeLists.txt` file. 
This can either be one of the top-level directories 
containing multiple projects, or one of the project-specific 
directories.

Specify a build directory for cmake and generate files for the build system:

```
~/ic4-examples/cpp/device-handling/device-enumeration $ cmake -B build/ .
-- The C compiler identification is GNU 13.2.0
-- The CXX compiler identification is GNU 13.2.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
(...)
-- Configuring done (1.3s)
-- Generating done (0.0s)
-- Build files have been written to: ~/ic4-examples/cpp/device-handling/device-enumeration/build
```

Now build the application:

```
~/ic4-examples/cpp/device-handling/device-enumeration $ cmake --build build/

[ 50%] Building CXX object CMakeFiles/device-enumeration.dir/src/device-enumeration.cpp.o
[100%] Linking CXX executable device-enumeration
[100%] Built target device-enumeration
```

Now run the example program:

```
~/ic4-examples/cpp/device-handling/device-enumeration $ ./build/device-enumeration
```

If everything worked, the program will now show that your camera was detected!


# Example Categories

The example programs are grouped by topic for clarity.

## Device Handling

These examples show how to
- [enumerate](/cpp/device-handling/device-enumeration) devices and interfaces
- Get [device-list-changed](/cpp/device-handling/device-list-changed/) notifications
- Handle [device-lost](/cpp/device-handling/device-lost) events

## Image Acquisition

This section contains example programs showing how to capture and
- Save images as [JPEG files](/cpp/image-acquisition/save-jpeg-file)
- Record videos as [H264-encoded MP4 files](/cpp/image-acquisition/record-mp4-h264) 
- Save [BMP files on trigger](/cpp/image-acquisition/save-bmp-on-trigger).

## Advanced Camera Features

Some cameras provide advanced features that can be utilized to solve specific application requirements. This section showcases
- Triggering multiple cameras simultaneously by broadcasting an [action command](/cpp/advanced-camera-features/action-command-broadcast-trigger)
- Reading camera-provided metadata from image buffers using [chunkdata](/cpp/advanced-camera-features/connect-chunkdata)
- Using [EventExposureEnd](/cpp/advanced-camera-features/event-exposure-end) to synchronize camera operation to real-world movement
- Get notified about I/O activity using [EventLine1*Edge](/cpp/advanced-camera-features/event-line1-edge) events

## Qt6

The Qt6 section provides a selection of pre-build dialogs that can speed up application development:

- The [PropertyDialog](/cpp/qt6/common/qt6-dialogs/PropertyDialog.h) class allows the user to quickly find and modify the features of a video capture device or other components.
- [DeviceSelectionDialog](/cpp/qt6/common/qt6-dialogs/DeviceSelectionDialog.h) is a dialog allowing device selection and configuration.

Several complete applications are also found here:

- [demoapp](/cpp/qt6/demoapp) contains the source code of the *ic4-demoapp* application distributed with the *IC Imaging Control4 SDK*.
- [device-manager](/cpp/qt6/device-manager) contains the source code of the *ic4-device-manager* application distributed with the *IC Imaging Control4 SDK*.
- [high-speed-capture](/cpp/qt6/high-speed-capture) is an example program showing how to use many image buffers to capture image data into memory for potentially slow processing tasks.

## Third-Party Integration

This section contains programs showing how to use data captured in `ImageBuffer` objects with third-party image processing libraries:

- [OpenCV](/cpp/thirdparty-integration/imagebuffer-opencv-snap)

## Win32/MFC

Contains a [small demo application](/cpp/win32-mfc/demoapp) using *MFC*. This example is using traditional *.sln* and *.vcxproj* files for *Visual Studio*.

## ic4-ctrl

This is the source code for the [ic4-ctrl](/cpp/ic4-ctrl) utility distributed with the *IC Imaging Control4 SDK*.
