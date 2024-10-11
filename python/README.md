# Python Examples for IC Imaging Control 4

This directory contains a selection of example programs demonstrating the use of the IC Imaging Control 4 Python API.

Most of the examples focus on a particular aspect of writing software to control a camera. Others, like [qt6/demoapp](python/qt6/demoapp) are small complete application programs.

## Download

To download all examples in a .zip file, move to the root of the repository, click the green *Code* button and select *Download ZIP*.

You can also use git from the command line to clone the examples.

After downloading, please make sure to keep the per-language directory structure intact so that relative file references can work.

***Warning:*** Be careful when cloning/extracting to a directory with a long path name; many tools are still limited by Windows' legacy 260-character path length limit and can raise obscure error messages in case a file path exceeds it.

## Prerequisites for Bulding and Testing the Example Programs

All examples assume you have access to an industrial camera from [The Imaging Source](www.theimagingsource.com).

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) matching your camera is installed.

All Python examples assume that the `imagingcontrol4` package is installed in the active environment. The Qt/PySide6 examples also require the `imagingcontrol4pyside6` package.

Some example programs use third-party packages like `numpy`, `opencv-python` and `PySide6`.

It is recommended (sometimes even enforced) to use virtual environments to separate different library installations from each other when working on multiple Python projects. For more information on how to create a virtual environment, see [Create a new Virtual Environment](https://packaging.python.org/en/latest/guides/installing-using-pip-and-virtual-environments/#create-a-new-virtual-environment).

After activating the environment, install the packages:

```
$ python3 -m pip install imagingcontrol4 imagingcontrol4pyside6
$ python3 -m pip install numpy opencv-python PySide6
```

## Running the Example Programs

With the prerequisites installed in the active environment, just run the example's main Python file.

# Example Categories

The example programs are grouped by topic for clarity.

## Device Handling

These examples show how to
- [enumerate](python/device-handling/device-enumeration) devices and interfaces
- Get [device-list-changed](python/device-handling/device-list-changed/) notifications
- Handle [device-lost](python/device-handling/device-lost) events

## Image Acquisition

This section contains example programs showing how to capture and
- Save images as [JPEG files](python/image-acquisition/save-jpeg-file/)
- Record videos as [H264-encoded MP4 files](python/image-acquisition/record-mp4-h264/)
- Save [BMP files on trigger](python/image-acquisition/save-bmp-on-trigger/)

## Advanced Camera Features

Some cameras provide advanced features that can be utilized to solve specific application requirements. This section showcases
- Triggering multiple cameras simultaneously by broadcasting an [action command](python/advanced-camera-features/actioncommand-broadcast-trigger)
- Reading camera-provided metadata from image buffers using [chunkdata](python/advanced-camera-features/connect-chunkdata)
- Using [EventExposureEnd](python/advanced-camera-features/EventExposureEnd) to synchronize camera operation to real-world movement

## Qt6

This section contains a simple application showing how to display live video in a Qt6/PySide6 application ([first steps](python/qt6/qt6-first-steps)) as well as a Python implementation of [ic4-demoapp](python/qt6/demoapp).

## Third-Party Integration

This section contains programs showing how to use data captured in `ImageBuffer` objects with third-party image processing libraries:

- [OpenCV Snap](python/thirdparty-integration/imagebuffer-numpy-opencv-live) shows how to pass images grabbed using ic4 to OpenCV.
- [OpenCV Live](python/thirdparty-integration/imagebuffer-numpy-opencv-live) shows how to manipulate images grabbed using ic4 using OpenCV, and displaying the result in an ic4 display.

## ic4-ctrl

This is a Python implementation of the `ic4-ctrl` utility distributed with the *IC Imaging Control4 SDK*.