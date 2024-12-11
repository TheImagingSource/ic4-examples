# .NET Examples for IC Imaging Control 4

This directory contains a selection of example programs demonstrating the use of the IC Imaging Control 4 .NET API.

## Download

To download all examples in a .zip file, move to the root of the repository, click the green *Code* button and select *Download ZIP*.

You can also select *Open with Visual Studio* to clone the examples into a directory on your computer, or use git directly.

After downloading, please make sure to keep the per-language directory structure intact so that relative file references can work.

***Warning:*** Be careful when cloning/extracting to a directory with a long path name; many tools are still limited by Windows' legacy 260-character path length limit and can raise obscure error messages in case a file path exceeds it.

## Prerequisites for Bulding and Testing the Example Programs

All examples assume you have access to an industrial camera from [The Imaging Source](https://www.theimagingsource.com).

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) matching your camera is installed.

All .NET examples can be used from *Visual Studio 2019* or later, or directly from the command line using the `dotnet` utility.

Console examples are usually built to be cross-platform and do not depend on Windows-specific libraries such as System.Windows.Forms.

Since the `ic4dotnet` and related packages are automatically downloaded from nuget.org, no SDK installation is required.

## Compiling and Running an Example Program from Visual Studio

Open the `.sln` file of the example program in Visual Studio and start the application.

## Compiling and Running an Example Program from the Command Line

Change into the directory of an example program (next to its `.csproj` file) and call:

```
$ dotnet run
```

# Example Categories

The example programs are grouped by topic for clarity.

## Device Handling

These examples show how to
- [enumerate](/dotnet/device-handling/DeviceEnumeration) devices and interfaces
- Get [device-list-changed](/dotnet/device-handling/DeviceListChanged/) notifications
- Handle [device-lost](/dotnet/device-handling/DeviceLost) events

## Image Acquisition

This section contains example programs showing how to capture and

- Save [BMP files on trigger](/dotnet/image-acquisition/SaveBmpOnTrigger).

## Advanced Camera Features

Some cameras provide advanced features that can be utilized to solve specific application requirements. This section showcases
- Triggering multiple cameras simultaneously by broadcasting an [action command](/dotnet/advanced-camera-features/ActionCommandBroadcastTrigger)
- Using [EventExposureEnd](/dotnet/advanced-camera-features/EventExposureEnd) to synchronize camera operation to real-world movement
- Get notified about I/O activity using [EventLine1*Edge](/dotnet/advanced-camera-features/EventLine1Edge) events

## Third-Party Integration

This section contains programs showing how to use data captured in `ImageBuffer` objects with third-party image processing libraries:

- [OpenCV](/dotnet/thirdparty-integration/ImageBufferOpenCVLive)

## Windows Forms

This section shows the Windows Forms integration of IC Imaging Control 4 using the `ic4dotnet.System.Windows.Forms` assembly. It contains:

- A simple Demo Application [for .NET Framework 4.5](/dotnet/winforms/framework45)
- A simple Demo Application [for .NET 6 or later](/dotnet/winforms/DialogApp-net6)
- An application showing how to draw a [Graphic Overlay](/dotnet/winforms/GraphicOverlay) on top of live video display

## Visual Basic .NET

This [example](/dotnet/vb.net/VB%20First%20Steps) shows how to use `ic4dotnet` using Visual Basic .NET.

## ic4-ctrl

This is a .NET implementation of the `ic4-ctrl` utility distributed with the *IC Imaging Control4 SDK*.