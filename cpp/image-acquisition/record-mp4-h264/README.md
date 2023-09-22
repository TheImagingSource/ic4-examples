
# Recording a Video File

## General Concept

A video file is created in 4 steps:

1. Create a `VideoWriter` object.
2. Call `VideoWriter::beginFile` to start a new video file.
3. Feed frames into the video file by repeatedly calling `VideoWriter::addFrame`.
4. Finally, call `VideoWriter::finishFile` to finalize the recording.

## Practical Implementation

### Step 0: Preparation

A video capture device is opened with the helper function from previous examples:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L73-L88

To get access to the image data received from the device, we have to create a sink object.
Since we want to record a video file of a portion of the data stream from the video capture device,
we need to access all image buffers that are received from the device during the time of recording.
The `QueueSink` is the perfect solution in this situation.

To use a `QueueSink`, a class has to be derived from `QueueSinkListener` that handles sink events, especially `QueueSinkListener::framesQueued`:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L10-L66

The sink is created by passing the listener instance to `QueueSink::create`:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L95-L104

The data stream is started by calling `Grabber::streamSetup`, passing int he newly-created sink:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L106-L111

### Step 1: Create a VideoWriter Object

A video writer is always created specifying the file type and encoder. In our example, we want to create MP4 files with H264 encoding:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L92-L93

### Step 2: Start a New Video File

A new video file is started by a call to `VideoWriter::beginFile`. The function expects a file name, the format of the buffers that are going to be fed into `VideoWriter::addFrame`, and
the video file's playback rate:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L143-L148

The required image type and frame rate information is gathered from the sink and device beforehand:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L113-L129

### Step 3: Add Frames to the Video File

In our queue sink's `framesQueued` callback function, the oldest available image buffer is extracted from the sink's output queue. Is important to
always pop and ultimately requeue the buffers to not starve the drivers of image buffers, even if the program does not always use the buffers:

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L27-L51

The function adds the image buffer to the current video file if the `do_write_frames_` flag indicates that a recording is active.

### Step 4: Finalize the Video File

https://github.com/TheImagingSource/ic4-examples/blob/6abd8de1f0556b918a2665e013a4c7338763a157/cpp/image-acquisition/record-mp4-h264/src/record-mp4-h264.cpp#L159-L164





