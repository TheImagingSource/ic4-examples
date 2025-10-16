# DoLP Segmentation

This sample shows to segment polarized light areas in a video image by Degree of Linear Polarization (DoLP)

## Prerequisites 
The example assumes you have access to polarization camera from [The Imaging Source](https://www.theimagingsource.com).
The supported polarization camera models are
* DYK 33GX250
* DZK 33GX250
* DYK 33UX250
* DZK 33UX250

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) (GigEVision or USB3Vision) matching your camera is installed.

The packages imagingcontrol4, imagingcontrol4pyside6, numpy and PySide6 are needed. They are installed by
```
$ pip install -r requirements.txt
```

## Description
The IC4 GenTL Producers provide "processed" polarization formats. Their structures are documented at [Polarized Formats](https://www.theimagingsource.com/en-us/documentation/ic4python/api-reference.html#imagingcontrol4.imagetype.PixelFormat.PolarizedADIMono8)

The formats used by the sample have following structure:
```C++
struct ADIMono8Pixel
{
    uint8_t AoLP; // Angle of Linear Polarization
    uint8_t DoLP; // Degree of Linear Polarization
    uint8_t Intensity; // Intensity
    uint8_t Reserved;
};
```
```C++
struct ADIMono8Pixel
{
    uint8_t AoLP; // Angle of Linear Polarization
    uint8_t DoLP_Red; // Degree of Linear Polarization of Red Light
    uint8_t DoLP_Green; // Degree of Linear Polarization of Green Light
    uint8_t DoLP_Blue; // Degree of Linear Polarization of Blue Light
    uint8_t Intensity_Red; // Intensity of Red Light
    uint8_t Intensity_Green; // Intensity of Green Light
    uint8_t Intensity_Blue; // Intensity of Blue Light
    uint8_t Reserved;
};
```
The numpy arrays created by the `ic4.ImageBuffer.numpy_wrap()` method use this structures for each pixel. 

If the DoLP values are greater than the DoLP threshold and the intensity (brightness) values are greater than the intensity threshold, the polarized light areas are marked with red pixels.






