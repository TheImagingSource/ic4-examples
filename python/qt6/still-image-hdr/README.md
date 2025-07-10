# HDR Still Image Capture

This sample shows how to capture two or four images with different exposure times and use OpenCV Merge-Mertens for creating an HDR image from these images.

## Prerequisites 
The example assumes you have access to an industrial camera from [The Imaging Source](https://www.theimagingsource.com).

Make sure a [GenTL Producer](https://www.theimagingsource.com/en-us/support/download/) matching your camera is installed.

The packages imagingcontrol4, imagingcontrol4pyside6, numpy, opencv-python and PySide6 are needed. They are installed by
```
$ pip install -r requirements.txt
```

## Description
HDR (High Dynamic Range) images are composed from a number of images, that are taken with different exposure times. In these images are areas that are overexposed as well underexposed. The goal is to compose a new image using all well exposed areas from the source images.

The program steps are:
- Disable camera automatics and get current exposure time.
- Calculate the different exposure times using the factor sliders and the current exposure time. 
- Capture images.
- -  Try to use multi frame output mode of the used camera. If that fails, use software trigger instead.
- Process the images into an HDR image. The OpenCV Fusion Mertens algorithm is used.
- Save and display the images.
- Enable camera automatics.

