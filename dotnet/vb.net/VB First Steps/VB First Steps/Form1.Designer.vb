<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.Display1 = New ic4.WinForms.Display()
        Me.btnDevice = New System.Windows.Forms.Button()
        Me.btnProperties = New System.Windows.Forms.Button()
        Me.btnSnap = New System.Windows.Forms.Button()
        Me.SuspendLayout()
        '
        'Display1
        '
        Me.Display1.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Display1.Location = New System.Drawing.Point(11, 11)
        Me.Display1.Margin = New System.Windows.Forms.Padding(2, 2, 2, 2)
        Me.Display1.Name = "Display1"
        Me.Display1.RenderHeight = 480
        Me.Display1.RenderLeft = 0
        Me.Display1.RenderPosition = ic4.DisplayRenderPosition.StretchCenter
        Me.Display1.RenderTop = 0
        Me.Display1.RenderWidth = 640
        Me.Display1.Size = New System.Drawing.Size(540, 354)
        Me.Display1.TabIndex = 0
        '
        'btnDevice
        '
        Me.btnDevice.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnDevice.Location = New System.Drawing.Point(555, 11)
        Me.btnDevice.Margin = New System.Windows.Forms.Padding(2, 2, 2, 2)
        Me.btnDevice.Name = "btnDevice"
        Me.btnDevice.Size = New System.Drawing.Size(69, 25)
        Me.btnDevice.TabIndex = 1
        Me.btnDevice.Text = "Device"
        Me.btnDevice.UseVisualStyleBackColor = True
        '
        'btnProperties
        '
        Me.btnProperties.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnProperties.Location = New System.Drawing.Point(555, 40)
        Me.btnProperties.Margin = New System.Windows.Forms.Padding(2, 2, 2, 2)
        Me.btnProperties.Name = "btnProperties"
        Me.btnProperties.Size = New System.Drawing.Size(69, 25)
        Me.btnProperties.TabIndex = 2
        Me.btnProperties.Text = "Properties"
        Me.btnProperties.UseVisualStyleBackColor = True
        '
        'btnSnap
        '
        Me.btnSnap.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnSnap.Location = New System.Drawing.Point(555, 69)
        Me.btnSnap.Margin = New System.Windows.Forms.Padding(2, 2, 2, 2)
        Me.btnSnap.Name = "btnSnap"
        Me.btnSnap.Size = New System.Drawing.Size(69, 25)
        Me.btnSnap.TabIndex = 3
        Me.btnSnap.Text = "Snap"
        Me.btnSnap.UseVisualStyleBackColor = True
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(635, 376)
        Me.Controls.Add(Me.btnSnap)
        Me.Controls.Add(Me.btnProperties)
        Me.Controls.Add(Me.btnDevice)
        Me.Controls.Add(Me.Display1)
        Me.Name = "Form1"
        Me.Text = "Visual Basic First Steps"
        Me.ResumeLayout(False)

    End Sub

    Friend WithEvents Display1 As ic4.WinForms.Display
    Friend WithEvents btnDevice As Button
    Friend WithEvents btnProperties As Button
    Friend WithEvents btnSnap As Button
End Class
