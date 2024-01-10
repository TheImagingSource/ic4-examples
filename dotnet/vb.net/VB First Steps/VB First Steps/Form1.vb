' This very simple examples shows how to:
' * How to initialize the IC4 library on a "main" form.
' * Open a video capture device.
' * Set the properties of a video capture device.
' * Use a snapsink for getting an image and save it to a file.
' * Restore a previously used device at program start.
'
' The solution platform must be x64. Otherwise, an error is shown.

Imports ic4

Public Class Form1
    Dim grabber As ic4.Grabber
    Dim snapsink As ic4.SnapSink
    ''' <summary>
    ''' The "New" sub for this for this form, which is our main form
    ''' must be overwritten, because the IC4 library must be initialized
    ''' before the IC4 display is created in InitializeComponents().
    ''' </summary>
    Public Sub New()
        ic4.Library.Init()
        InitializeComponent() ' This call is required by the Windows Form Designer.
    End Sub

    ''' <summary>
    ''' Initialize the IC4 grabber and the IC4 snapsink. 
    ''' Try to open the last used camera and start the stream.
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        ' Create the snapsink, which is used to snap a frame
        ' from the stream into a frame buffer.
        snapsink = New ic4.SnapSink

        ' Create the grabber object, which handles the camera
        grabber = New ic4.Grabber

        ' Try to restore the last used device
        Try
            grabber.DeviceOpenFromState("device.xml")
            grabber.StreamSetup(snapsink, Display1)
        Catch ex As Exception
            Console.WriteLine("Last used device not found.")
        End Try
    End Sub

    ''' <summary>
    ''' Clicking on the Device Button shows the IC4 device selection 
    ''' dialog. 
    ''' A new selected device is saved in the device state xml file. 
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    Private Sub btnDevice_Click(sender As Object, e As EventArgs) Handles btnDevice.Click
        If grabber.IsStreaming Then
            grabber.StreamStop()
        End If
        ic4.WinForms.Dialogs.ShowDeviceDialog(grabber, Me)
        If grabber.IsDeviceValid Then
            grabber.DeviceSaveState("device.xml")
            ' Start the stream.
            grabber.StreamSetup(snapsink, Display1)
        End If

    End Sub

    ''' <summary>
    ''' Show the IC4 property dialog of a camera. In this dialog are the
    ''' video format (pixelformat), frame rate and all other properties
    ''' set.
    ''' The new settings are saved in the device state xml file.
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    Private Sub btnProperties_Click(sender As Object, e As EventArgs) Handles btnProperties.Click
        ' The flag ic4.WinForms.PropertyDialogFlags.AllowStreamRestart is
        ' needed in order to set the pixel format and acquisition framerate,
        ' while the stream is running. 

        If grabber.IsDeviceValid Then
            If ic4.WinForms.Dialogs.ShowDevicePropertyDialog(grabber, Me, ic4.WinForms.PropertyDialogFlags.AllowStreamRestart) Then
                grabber.DeviceSaveState("device.xml")
            End If
        Else
            MessageBox.Show("Please open a device first!")
        End If
    End Sub

    ''' <summary>
    ''' Save an image using the snapsink. The image is saved in the working
    ''' directory in order to keep this sample simple. 
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    Private Sub btnSnap_Click(sender As Object, e As EventArgs) Handles btnSnap.Click
        Dim buffer As ic4.ImageBuffer
        If grabber.IsStreaming Then
            Try
                buffer = snapsink.SnapSingle(TimeSpan.FromSeconds(1))
                buffer.SaveAsJpeg("snap.jpg", 70)
            Catch ex As Exception
                MessageBox.Show("No image received: " + ex.Message)
            End Try
        End If
    End Sub
End Class
