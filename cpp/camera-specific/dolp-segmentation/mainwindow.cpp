/*
 * Copyright The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "mainwindow.h"

#include "DeviceSelectionDialog.h"
#include "PropertyDialog.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct BGRa8
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
};

struct PolarizedADIMono8
{
	uint8_t AoLP;
	uint8_t DoLP;
	uint8_t Intensity;
	uint8_t Reserved;
};

struct PolarizedADIRGB8
{
	uint8_t AoLP;
	uint8_t DoLPRed;
	uint8_t DoLPGreen;
	uint8_t DoLPBlue;
	uint8_t IntensityRed;
	uint8_t IntensityGreen;
	uint8_t IntensityBlue;
	uint8_t Reserved;
};



MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	createUI();

	checkForGenTLProducers();

	// Create the sink for accessing images.
	_queueSink = ic4::QueueSink::create(*this);

	// Create a buffer pool to allocate visualization buffers from
	_bufferPool = ic4::BufferPool::create(ic4::BufferPool::CacheConfig{ 2, 0 });

	// Create the display for the live video
	try
	{
		_display = _VideoWidget->asDisplay();
		_display->setRenderPosition(ic4::DisplayRenderPosition::StretchCenter);
	}
	catch (const ic4::IC4Exception& ex)
	{
		QMessageBox::information(this, {}, ex.what());
	}

	startstopstream();
	updateControls();
}

MainWindow::~MainWindow()
{
}

/// <summary>
/// Create the user interface of the demo app
/// </summary>
void MainWindow::createUI()
{
	resize(1024, 768);

	// Create mainwidget
	auto mainWidget = new QWidget();
	//Create the main layout
	auto mainlayout = new QHBoxLayout();

	// Create the video display Widget
	_VideoWidget = new ic4interop::Qt::DisplayWidget(this);
	_VideoWidget->setMinimumSize(640, 480);
	mainlayout->addWidget(_VideoWidget);

	auto sideLayout = new QVBoxLayout(this);

	_btnDevice = new QPushButton("Device");
	_btnProperties = new QPushButton("Properties");
	_btnLiveVideo = new QPushButton("Start");
	_sldThresholdDoLP = new SliderControl("Threshold DoLP", 30, 0, 255, this);
	_sldThresholdIntensity = new SliderControl("Threshold Intensity", 10.0, 0.0, 255.0, this);

	_btnDevice->setMaximumWidth(100);
	_btnProperties->setMaximumWidth(100);
	_btnLiveVideo->setMaximumWidth(100);
	_sldThresholdDoLP->setMaximumWidth(250);
	_sldThresholdIntensity->setMaximumWidth(250);

	connect(_btnDevice, &QPushButton::pressed, this, &MainWindow::onSelectDevice);
	connect(_btnProperties, &QPushButton::pressed, this, &MainWindow::onDeviceProperties);
	connect(_btnLiveVideo, &QPushButton::pressed, this, &MainWindow::startstopstream);

	connect(_sldThresholdDoLP, &SliderControl::sliderValueChanged, this, &MainWindow::onThresholdDoLPChanged);
	connect(_sldThresholdIntensity, &SliderControl::sliderValueChanged, this, &MainWindow::onThresholdIntensityChanged);

	sideLayout->setAlignment(Qt::AlignTop);
	sideLayout->addWidget(_btnDevice);
	sideLayout->addWidget(_btnProperties);
	sideLayout->addWidget(_btnLiveVideo);
	sideLayout->addWidget(_sldThresholdDoLP);
	sideLayout->addWidget(_sldThresholdIntensity);

	mainlayout->addLayout(sideLayout);

	mainWidget->setLayout(mainlayout);

	setCentralWidget(mainWidget);
}

/// <summary>
/// Check, whether IC4 GenTL Producers are installed. Without a GenTL Producer,
/// the example program won't detect a camera.
/// </summary>
/// <returns>True on GenTL producers installed.</returns>
bool MainWindow::checkForGenTLProducers()
{
	auto res = ic4::getVersionInfo(ic4::VersionInfoFlags::Driver);
	if (ic4::DeviceEnum().enumInterfaces().size() == 0)
	{
		QMessageBox::warning(
			this, {},
			"It is possible that no IC4 GenTL Producers are installed.\nPlease install from\nhttps://www.theimagingsource.com/en-us/support/download/"
		);
		return false;
	}
	return true;
}

/// <summary>
/// On closing the application:
/// - Stop the live stream.
/// - Close the the device.
/// </summary>
/// <param name="event"></param>
void MainWindow::closeEvent(QCloseEvent* event)
{
	if (_grabber.isDeviceValid())
	{
		if (_grabber.isStreaming())
		{
			_grabber.streamStop();
		}
		_grabber.deviceClose();
	}
}

/// <summary>
/// Change the controls' states depending on device states.
/// </summary>
void MainWindow::updateControls()
{
	if (_grabber.isStreaming())
	{
		_btnLiveVideo->setText("Stop");
	}
	else
	{
		_btnLiveVideo->setText("Start");
	}

	if (_grabber.isDeviceValid())
	{
		_btnLiveVideo->setEnabled(true);
		_btnProperties->setEnabled(true);
	}
	else
	{
		_btnLiveVideo->setEnabled(false);
		_btnProperties->setEnabled(false);
		_btnLiveVideo->setText("Start");
	}
}

/// <summary>
/// Show the device selection dialog for selecting a camera.
/// </summary>
void MainWindow::onSelectDevice()
{
	DeviceSelectionDialog cDlg(this, &_grabber);

	if (cDlg.exec() == QDialog::Accepted)
	{
		if (isPolarizationCamera())
		{
			setWindowTitle(QString::fromStdString(_grabber.deviceInfo(ic4::Error::Ignore()).modelName()));
			setPolarizationFormat();
			startstopstream();
		}
		else
		{
			QMessageBox::warning(this, {}, "No polarization camera (DYK or DZK) selected.");
			setWindowTitle({});
			_grabber.deviceClose();
		}
	}
	updateControls();
}

bool MainWindow::isPolarizationCamera()
{
	ic4::Error err;
	auto propPixelFormat = _grabber.devicePropertyMap(err).find(ic4::PropId::PixelFormat, err);
	auto pixelFormatEntries = propPixelFormat.entries(err);

	// Determine camera type by looking for polarized pixel formats
	auto it = std::find_if(pixelFormatEntries.begin(), pixelFormatEntries.end(),
		[](auto& e)
		{
			auto fmt = static_cast<ic4::PixelFormat>(e.intValue(ic4::Error::Ignore()));
			return fmt == ic4::PixelFormat::PolarizedMono8 || fmt == ic4::PixelFormat::PolarizedBayerBG8;
		}
	);
	return it != pixelFormatEntries.end();
}

/// <summary>
/// Enable the "Processed Pixel Formats" in the IC4 GenTL Producer.
/// Try to set the ADIMono8 Format. This is successful if a monochrome
/// camera is connected. If that fails, there is color camera connected
/// and the ADIRGB8 Format is used.
/// </summary>
/// <returns>True on success</returns>
bool MainWindow::setPolarizationFormat()
{
	_grabber.devicePropertyMap().setValue("ProcessedPixelFormatsEnable", true);
	try
	{
		if (_grabber.devicePropertyMap().setValue(ic4::PropId::PixelFormat, ic4::PixelFormat::PolarizedADIMono8))
		{
			return true;
		}
	}
	catch (ic4::IC4Exception e)
	{
		try
		{
			if (_grabber.devicePropertyMap().setValue(ic4::PropId::PixelFormat, ic4::PixelFormat::PolarizedADIRGB8))
			{
				return true;
			}
		}
		catch (ic4::IC4Exception e)
		{
			QMessageBox::warning(this, {}, e.what());
		}
	}
	return false;
}

/// <summary>
/// Show the properties dialog of a device for e.g. exposure and gain adjustment
/// </summary>
void MainWindow::onDeviceProperties()
{
	PropertyDialog cDlg(_grabber, this, tr("Device Properties"));
	cDlg.exec();
}

/// <summary>
/// Start and stop the live video stream
/// </summary>
void MainWindow::startstopstream()
{
	try
	{
		if (_grabber.isDeviceValid())
		{
			if (_grabber.isStreaming())
			{
				_grabber.streamStop();
			}
			else
			{
				// Stream is set up without a display
				// Processed images are displayed manually from the sink's framesQueued callback.
				_grabber.streamSetup(_queueSink);
			}
		}
	}
	catch (ic4::IC4Exception ex)
	{
		QMessageBox::warning(this, {}, ex.what());
	}

	updateControls();
}

/// <summary>
/// Event handler for the DoLP threshold slider
/// </summary>
/// <param name="value">New threshold value</param>
void MainWindow::onThresholdDoLPChanged(int value)
{
	_ThresholdDoLP = value;
}

/// <summary>
/// Event handler for the intensity threshold slider
/// </summary>
/// <param name="value">New threshold value</param>
void MainWindow::onThresholdIntensityChanged(int value)
{
	_ThresholdIntensity = value;
}

/// <summary>
/// Listener related. Allocate image buffers.
/// </summary>
/// <param name="sink"></param>
/// <param name="imageType"></param>
/// <param name="min_buffers_required"></param>
/// <returns>Always true</returns>
bool MainWindow::sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required)
{
	// Allocate more buffers than suggested, because we temporarily take some buffers
	// out of circulation when saving an image or video files.
	sink.allocAndQueueBuffers(min_buffers_required + 2);
	return true;
};

/// <summary>
/// Listener related: Callback for new frames.
/// </summary>
/// <param name="sink"></param>
void MainWindow::framesQueued(ic4::QueueSink& sink)
{
	// Get current buffer
	auto buffer = sink.popOutputBuffer();

	// Create destination buffer for transformation
	auto dest_buffer = _bufferPool->getBuffer(buffer->imageType().with_pixel_format(ic4::PixelFormat::BGRa8));

	// Depending on the source buffer type, call the correct visualization function
	if (buffer->imageType().pixel_format() == ic4::PixelFormat::PolarizedADIMono8)
	{
		ThresholdPolarizedADIMono8(*buffer, *dest_buffer);
	}
	else
	{
		ThresholdPolarizedADIRGB8(*buffer, *dest_buffer);
	}

	// Manually display buffer
	_display->displayBuffer(dest_buffer);
}

/// <summary>
/// Copy pixels from source to destination and mark all pixels with polarized light in red.
/// </summary>
/// <param name="src">Polarized mono8 image buffer</param>
/// <param name="dest">RGB32 (BGRa) output image buffer</param>
void MainWindow::ThresholdPolarizedADIMono8(const ic4::ImageBuffer& src, ic4::ImageBuffer& dest)
{
	auto width = src.imageType().width();
	auto height = src.imageType().height();

	auto src_ptr = static_cast<const uint8_t*>(src.ptr());
	auto src_pitch = src.pitch();
	auto dst_ptr = static_cast<uint8_t*>(dest.ptr());
	auto dst_pitch = dest.pitch();

	for (int y = 0; y < height; y++)
	{
		auto pSrcLine = reinterpret_cast<const PolarizedADIMono8*>(src_ptr + y * src_pitch);
		auto pDestLine = reinterpret_cast<BGRa8*>(dst_ptr + y * dst_pitch);

		for (int x = 0; x < width; x++)
		{
			if (pSrcLine[x].DoLP > _ThresholdDoLP && pSrcLine[x].Intensity > _ThresholdIntensity)
			{
				pDestLine[x].Blue = 0x00;
				pDestLine[x].Green = 0x00;
				pDestLine[x].Red = 0xFF;
				pDestLine[x].Alpha = 0xFF;
			}
			else
			{
				pDestLine[x].Blue = pSrcLine[x].Intensity;
				pDestLine[x].Green = pSrcLine[x].Intensity;
				pDestLine[x].Red = pSrcLine[x].Intensity;
				pDestLine[x].Alpha = 0xFF;
			}
		}
	}
}
/// <summary>
/// Copy pixels from source to destination and mark all pixels with polarized light in red.
/// </summary>
/// <param name="src">Polarized BGR8 image buffer</param>
/// <param name="dest">RGB32 (BGRa) output image buffer</param>
void MainWindow::ThresholdPolarizedADIRGB8(const ic4::ImageBuffer& src, ic4::ImageBuffer& dest)
{
	auto width = src.imageType().width();
	auto height = src.imageType().height();

	auto src_ptr = static_cast<const uint8_t*>(src.ptr());
	auto src_pitch = src.pitch();
	auto dst_ptr = static_cast<uint8_t*>(dest.ptr());
	auto dst_pitch = dest.pitch();

	for (int y = 0; y < height; y++)
	{
		auto pSrcLine = reinterpret_cast<const PolarizedADIRGB8*>(src_ptr + y * src_pitch);
		auto pDestLine = reinterpret_cast<BGRa8*>(dst_ptr + y * dst_pitch);

		for (int x = 0; x < width; x++)
		{
			int avgDoLP = (pSrcLine[x].DoLPRed + pSrcLine[x].DoLPGreen + pSrcLine[x].DoLPBlue) / 3;
			int avgIntensity = (pSrcLine[x].IntensityRed + pSrcLine[x].IntensityGreen + pSrcLine[x].IntensityBlue) / 3;
			if (avgDoLP > _ThresholdDoLP && avgIntensity > _ThresholdIntensity)
			{
				pDestLine[x].Blue = 0x00;
				pDestLine[x].Green = 0x00;
				pDestLine[x].Red = 0xFF;
				pDestLine[x].Alpha = 0xFF;
			}
			else
			{
				pDestLine[x].Blue = pSrcLine[x].IntensityBlue;
				pDestLine[x].Green = pSrcLine[x].IntensityGreen;
				pDestLine[x].Red = pSrcLine[x].IntensityRed;
				pDestLine[x].Alpha = 0xFF;
			}
		}
	}
}
