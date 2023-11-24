
# Save BMP on Trigger

## General Concept

In many situations, the camera is required to take an image exactly at the time of an external event.
In this scenario, the camera can be put into trigger mode. In trigger mode, the camera does not take images, but instead
waits for a trigger signal. Multiple sources can be used as the trigger signal, e.g. a digital input, a software signal,
or even a specifically crafted network packet.

A program using trigger mode usually wants to capture all images sent by the device, and requires notifications once
an image is received. For this purpose, using a queue sink is the optimal solution.
It maintains two queues: A free queue of image buffers yet to be filled, and an output queue of completed buffers ready to be processed.
A callback function is called whenever a buffer was filled and added to the output queue.

In this example, we use a queue sink to be notified of incoming images and save them as bitmap files.
For demonstration purposes (since not everyone has the hardware for digital input available) the program uses the camera's software trigger feature
to issue trigger signals to the camera.


## Practical Implementation

### Step 0: Preparation

A video capture device is opened with the helper function from previous examples:

https://github.com/TheImagingSource/ic4-examples/blob/36fd790f220ae28ad7567f7c84caa6920f8bf654/cpp/image-acquisition/save-bmp-on-trigger/src/save-bmp-on-trigger.cpp#L60-L75


### Step 1: Configure the Device

Next, the device needs to be configured. For simplicity, we just load a UserSet to restore all camera settings to factory defaults.
`TriggerSelector` is set to FrameStart to trigger images. After that, trigger mode is enabled by setting `TriggerMode` to On:

https://github.com/TheImagingSource/ic4-examples/blob/36fd790f220ae28ad7567f7c84caa6920f8bf654/cpp/image-acquisition/save-bmp-on-trigger/src/save-bmp-on-trigger.cpp#L77-L97


### Step 2: Define a QueueSinkListener

To get notifications from a queue sink, a class has to be derived from `QueueSinkListener`, overriding the `framesQueued` method.

In the `framesQueued` function, image buffers are extracted from the sink's output queue using `popOutputBuffer`, and then saved using `imageBufferSaveAsBitmap`:

https://github.com/TheImagingSource/ic4-examples/blob/36fd790f220ae28ad7567f7c84caa6920f8bf654/cpp/image-acquisition/save-bmp-on-trigger/src/save-bmp-on-trigger.cpp#L8-L53


### Step 3: Start the Stream to the Sink

Next, we create a new QueueSink using `QueueSink::create`, passing a reference an instance of the `QueueSinkListener` defined above.

After that, the a data stream to the sink is started using `Grabber::streamSetup`:

https://github.com/TheImagingSource/ic4-examples/blob/36fd790f220ae28ad7567f7c84caa6920f8bf654/cpp/image-acquisition/save-bmp-on-trigger/src/save-bmp-on-trigger.cpp#L99-L116


### Step 4: Waiting for Images

We set up a loop to let the program run while waiting for images. Whenever a key is pressed, a `TriggerSoftware` command is executed to trigger an image:

https://github.com/TheImagingSource/ic4-examples/blob/36fd790f220ae28ad7567f7c84caa6920f8bf654/cpp/image-acquisition/save-bmp-on-trigger/src/save-bmp-on-trigger.cpp#L126-L138

