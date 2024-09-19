# C++ Examples for IC Imaging Control 4

This directory contains a selection of example programs demonstrating the use of the IC Imaging Control 4 C++ API.

Most of the examples focus on a particular aspect of writing software to control a camera. Others, like [qt6/demoapp](qt6/demoapp) are small complete application programs.

# Prerequisites for Using the Example Programs

All examples assume you have access to an industrial camera from [The Imaging Source](www.theimagingsource.com).

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) matching your camera is installed.

All C++ examples assume that the IC4 SDK for C++ is installed. For Windows, this means installing *IC Imaging Control 4 SDK* from the [website](https://www.theimagingsource.com/en-us/support/download/). For Linux, at least the *ic4-dev* package is required.

The examples require cmake (On Windows, this is provided by Visual Studio 2019 or later) and a C++ compiler supporting at least C++14.

# Compiling the Example Programs

Most examples are built using *CMake*. You can either manually use *CMake* to build the example programs, or use an IDE that knows how to build *CMake* projects.
*Visual Studio* natively support *CMake*' projects from version 2019 onwards.

## Using Visual Studio

From the main menu, select *File -> Open... -> Folder...* and navigate to a directory containing a `CMakeLists.txt` file.

After the project was configured successfully, select the executable you want to run from *Select Startup Item...* dropdown next to the *Start Debugging* button.

Click *Start Debugging* and the program will be compiled and run.

## Using cmake from the Command Line

First, enter a directory containing a `CMakeLists.txt` file. This can either be one of
the top-level directories containing multiple projects, or one of the project-specific directories.

Then, create a `build` directory and change into it:

```
~/ic4-examples/cpp/device-handling/device-enumeration $ mkdir build
~/ic4-examples/cpp/device-handling/device-enumeration $ cd build
```

In the build directory, run cmake pointing to the parent directory:

```
~/ic4-examples/cpp/device-handling/device-enumeration/build $ cmake ..

-- The C compiler identification is GNU 13.2.0
-- The CXX compiler identification is GNU 13.2.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
(...)
-- Configuring done (1.3s)
-- Generating done (0.0s)
-- Build files have been written to: ~/ic4-examples/cpp/device-handling/device-enumeration/build
```

CMake will now generate files for the default build system, e.g. a MakeFile. Run `make` to build the example program

```
~/ic4-examples/cpp/device-handling/device-enumeration/build $ make

[ 50%] Building CXX object CMakeFiles/device-enumeration.dir/src/device-enumeration.cpp.o
[100%] Linking CXX executable device-enumeration
[100%] Built target device-enumeration
```

Now run the example program:

```
~/ic4-examples/cpp/device-handling/device-enumeration/build $ ./device-enumeration.cpp
```

If everything worked, the program will now show that your camera was detected!


# Example Categories

The example programs are grouped by topic for clarity.

## Device Handling

These examples show how to [enumerate devices](device-handling/device-enumeration), get device-list-changed [notifications](device-handling/device-list-changed/)
and handling [device-lost events](device-handling/device-lost).

## Image Acquisition

This section contains example programs showing how to capture and save images as [JPEG files](image-acquisition/save-jpeg-file),
record videos as [H264-encoded MP4 files](image-acquisition/record-mp4-h264) or save [BMP files on trigger](image-acquisition/save-bmp-on-trigger).

## Advanced Camera Features

## Qt6

## Third-Party Integration

## Win32/MFC

## ic4-ctrl

## Common