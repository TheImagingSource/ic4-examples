# Graphic Overlay 
This example demonstrates how graphics can be drawn on the live video and displayed in IC Imaging Control's `ic4.WinForms.Display`.



## Prerequisites
- The Imaging Source camera or video capture device
- IC4 GenTL Producer for the above mentioned device
- Visual Studio&trade; C#, .NET 6

## Implementation
The program displays a red line on top of each image coming from the camera.

To do this, an `ic4.QueueSink` is created that raises an event for every new image.

The event handler calls `QueueSink.PopOutputBuffer()` to retrieve the image buffer.

The `ic4dotnet.System.Drawing` assembly provides the extension method `ImageBuffer.CreateBitmapWrap()`
which creates a `System.Drawing.Bitmap` that shares the image buffer's memory.

To draw the overlay, a `System.Drawing.Graphics` object is created for the bitmap. Drawing operations
on this object modify the contents of the image buffer.

Finally, the modified image buffer is passed to an `ic4.WinForms.Display` control for display.

The drawing takes place in the callback of the IC4 `QueueSink`, as the drawing must be repeated for each new image. 

### Setup the QueueSink
In the `startStream` method the sink is created:
```C#
sink = new ic4.QueueSink(ic4.PixelFormat.BGRa8);
```
`Graphics.FromImage()` only supports RGB pixel formats. We therefore request a conversion to
`BGRa8` to make sure System.Drawing can draw into the image buffer. (BGR8 would be possible as well.)

Then register the `FramesQueued` event handler to this sink: 
```C#
sink.FramesQueued += sink_FramesQueued;
```
The complete start method is:
```C#
void startStream()
{
    if (!grabber.IsStreaming)
    {
        // Create the sink for continuous image processing.
        // We want to use BGRa8 (RGB32) in the sink, which 
        // allows us to use System.Drawing.Graphics for drawing.                
        try
        {
            var sink = new ic4.QueueSink(ic4.PixelFormat.BGRa8);
            sink.FramesQueued += sink_FramesQueued;

            grabber.StreamSetup(sink );   
            
        }
        catch( IC4Exception iex)
        {
            MessageBox.Show($"Error starting stream:\n{iex.Message}", 
                "Graphic Overlay",
                MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
    updateControls();
}
```
Please note: The `ic4.WinForms.Display` display object ic4Display is not passed to StreamSetup,
since the modified image buffers are passed manually in the `QueueSink.FramesQueued` event handler.
If the data stream displayed automatically the camera images as well, the display could flicker 
between modified and unmodified image buffers.

### Drawing on the live video

```C#
private void sink_FramesQueued(object sender, QueueSinkEventArgs e)
{
    using var buffer = e.Sink.PopOutputBuffer();

    // Create a System.Drawing.Bitmap which shares the memory
    // of the image buffer.                
    using var bitmap = buffer.CreateBitmapWrap();

    // Now draw a line.
    using var g = Graphics.FromImage(bitmap);
    g.DrawLine(new Pen(Color.Red, 3), 10, 10, 200, 200);

    // Now draw the image with the line in IC Imaging Display Control
    // This works fine, because buffer and bitmap share the same
    // memory. 
    ic4Display.DisplayBuffer(buffer);
}
```