/*
 * Copyright 2015 bvtest <email>
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
#include <QApplication>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QIcon>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QStandardPaths>

#include <string>
#include <iostream>
#include "events.h"

#include "ic4dialogs/deviceselection/deviceselection.h"
#include "ic4dialogs/propertydlg/propertydlg.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	_name("IC4 Demo App"),
	_shootPhoto(false),
	_videowriter(ic4::VideoWriterType::MP4_H264),
	_capturetovideo(false),
	_videocapturepause(false)
{
	this->setWindowTitle(_name.c_str());
	createUI();

	// Make sure the %appdata%/demoapp directory exists
	auto appDataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir(appDataDirectory).mkdir(".");

	_devicefile = appDataDirectory.toStdString() + "/device.json";
	_codecconfigfile = appDataDirectory.toStdString() + "/codecconfig.json";

	// Create the sink for accessing images.
	_queuesink = ic4::QueueSink::create(*this);

	// Add a device lost handler. 
	_grabber.eventAddDeviceLost([this](ic4::Grabber& g) {
		QApplication::postEvent(this, new QEvent(DEVICE_LOST_EVENT));
	});

	// Create the display for the live video
	try
	{
		WId wid = _VideoWidget->winId();
		_display = ic4::Display::create(ic4::DisplayType::Default, (ic4::WindowHandle)wid);
		_display->setRenderPosition(ic4::DisplayRenderPosition::StretchCenter);
	}
	catch (const ic4::IC4Exception& ex)
	{
		QMessageBox::information(NULL, _name.c_str(), ex.what());
	}

	if (QFileInfo::exists(_devicefile.c_str()))
	{
		ic4::Error err;

		// Try to load the last used device.
		if (!_grabber.deviceOpenFromState(_devicefile, err))
		{
			auto message = "Loading last used device failed: " + err.message();
			QMessageBox::information(NULL, _name.c_str(), message.c_str());
		}

		updateCameraLabel();
	}


	if (QFileInfo::exists(_codecconfigfile.c_str()))
	{
		ic4::Error err;

		try
		{
			_videowriter.getPropertyMap().deSerialize(_codecconfigfile, err);
		}
		catch (const ic4::IC4Exception& ex)
		{
			auto message = "Loading last codec configuration failed: " + std::string(ex.what());
			QMessageBox::information(NULL, _name.c_str(), message.c_str());
		}
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
	////////////////////////////////////////////////////////////////////////////
	// Define program actions

	// Device Selection
	_DeviceSelectAct = new QAction(QIcon(":/images/camera.png"), tr("&Select"), this);
	_DeviceSelectAct->setStatusTip(tr("Select a video capture device"));
	connect(_DeviceSelectAct, &QAction::triggered, this, &MainWindow::onSelectDevice);

	// Device Properties
	_DevicePropertiesAct = new QAction(QIcon(":/images/imgset.png"), tr("&Properties"), this);
	_DevicePropertiesAct->setStatusTip(tr("Show device property dialog"));
	_DevicePropertiesAct->setEnabled(false);
	connect(_DevicePropertiesAct, &QAction::triggered, this, &MainWindow::onDeviceProperties);

	// Trigger Mode
	_TriggerModeAct = new QAction(QIcon(":/images/triggermode.png"), tr("&Trigger mode"), this);
	_TriggerModeAct->setStatusTip(tr("Enable and disable trigger mode"));
	_TriggerModeAct->setCheckable(true);
	_TriggerModeAct->setEnabled(false);
	_TriggerModeAct->setChecked(false);
	connect(_TriggerModeAct, &QAction::triggered, this, &MainWindow::onToggleTriggerMode);

	// Start/Stop Live Stream
	_StartLiveAct = new QAction(QIcon(":/images/livestream.png"), tr("&Live stream"), this);
	_StartLiveAct->setStatusTip(tr("Start and stop the live stream"));
	_StartLiveAct->setCheckable(true);
	_StartLiveAct->setEnabled(false);
	_StartLiveAct->setChecked(false);
	connect(_StartLiveAct, &QAction::triggered, this, &MainWindow::startstopstream);

	// Capture Photo
	_ShootPhotoAct = new QAction(QIcon(":/images/photo.png"), tr("&Shoot photo"), this);
	_ShootPhotoAct->setStatusTip(tr("Shoot and save a photo"));
	_ShootPhotoAct->setEnabled(false);
	connect(_ShootPhotoAct, &QAction::triggered, this, &MainWindow::onShootPhoto);

	// Capture Video
	_recordstartact = new QAction(QIcon(":/images/recordstart.png"), tr("&Capture video"), this);
	_recordstartact->setStatusTip(tr("Capture video into MP4 file"));
	_recordstartact->setCheckable(true);
	connect(_recordstartact, &QAction::triggered, this, &MainWindow::onStartCaptureVideo);

	// Pause Video Recording
	_recordpauseact = new QAction(QIcon(":/images/recordpause.png"), tr("&Pause capture video"), this);
	_recordpauseact->setStatusTip(tr("Pause video capture"));
	_recordpauseact->setCheckable(true);
	_recordpauseact->setEnabled(true);
	_recordpauseact->setChecked(false);
	connect(_recordpauseact, &QAction::triggered, this, &MainWindow::onPauseCaptureVideo);

	// Stop Video Recording
	_recordstopact = new QAction(QIcon(":/images/recordstop.png"), tr("&Stop capture video"), this);
	_recordstopact->setStatusTip(tr("End capture video into MP4 file"));
	_recordstopact->setEnabled(false);
	connect(_recordstopact, &QAction::triggered, this, &MainWindow::onStopCaptureVideo);

	// Codec Properties
	_codecpropertyact = new QAction(QIcon(":/images/gear.png"), tr("&Codec properties"), this);
	_codecpropertyact->setStatusTip(tr("Configure the video codec"));
	connect(_codecpropertyact, &QAction::triggered, this, &MainWindow::onCodecProperties);

	// Exit Program
	auto exitAct = new QAction(tr("&Exit"), this);
	exitAct->setShortcuts(QKeySequence::Close);
	exitAct->setStatusTip(tr("Exit program"));
	connect(exitAct, &QAction::triggered, this, &QWidget::close);

	////////////////////////////////////////////////////////////////////////////
	// Create the File Menu
	auto fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(exitAct);
	////////////////////////////////////////////////////////////////////////////
	// Create the Device Menu
	auto deviceMenu = menuBar()->addMenu(tr("&Device"));
	deviceMenu->addAction(_DeviceSelectAct);
	deviceMenu->addAction(_DevicePropertiesAct);
	deviceMenu->addAction(_TriggerModeAct);
	deviceMenu->addAction(_StartLiveAct);
	////////////////////////////////////////////////////////////////////////////
	// Create the Capture Menu
	auto captureMenu = menuBar()->addMenu(tr("&Capture"));
	captureMenu->addAction(_ShootPhotoAct);
	captureMenu->addAction(_recordstartact);
	captureMenu->addAction(_recordpauseact);
	captureMenu->addAction(_recordstopact);
	captureMenu->addAction(_codecpropertyact);

	////////////////////////////////////////////////////////////////////////////
	// Create the Toolbar
	QToolBar* toolbar = new QToolBar(this);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);
	toolbar->addAction(_DeviceSelectAct);
	toolbar->addAction(_DevicePropertiesAct);
	toolbar->addSeparator();
	toolbar->addAction(_TriggerModeAct);
	toolbar->addSeparator();
	toolbar->addAction(_StartLiveAct);
	toolbar->addSeparator();
	toolbar->addAction(_ShootPhotoAct);
	toolbar->addSeparator();
	toolbar->addAction(_recordstartact);
	toolbar->addAction(_recordpauseact);
	toolbar->addAction(_recordstopact);
	toolbar->addAction(_codecpropertyact);

	////////////////////////////////////////////////////////////////////////////
	// Create the video display Widget
	_VideoWidget = new QWidget();
	_VideoWidget->setMinimumSize(640, 480);
	setCentralWidget(_VideoWidget);

	statusBar()->showMessage(tr("Ready"));
	_sbCameralabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(_sbCameralabel);
}

/////////////////////////////////////////////////////////////
// Event handler
void MainWindow::customEvent(QEvent* event)
{
	// When we get here, we've crossed the thread boundary and are now
	// executing in the MainWindow's thread. We could ask more events here.
	if (event->type() == GOT_PHOTO_EVENT)
	{
		const auto& gotPhotoEvent = *static_cast<GotPhotoEvent*>(event);
		savePhoto(gotPhotoEvent.getImageBuffer());
	}
	else if (event->type() == DEVICE_LOST_EVENT)
	{
		onDeviceLost();
	}

	// use more else ifs to handle other custom events
}

/// <summary>
/// Change the controls' states depending on device states.
/// </summary>
void MainWindow::updateControls()
{
	_DevicePropertiesAct->setEnabled(_grabber.isDeviceValid());
	_StartLiveAct->setEnabled(_grabber.isDeviceValid());
	_StartLiveAct->setChecked(_grabber.isStreaming());
	_ShootPhotoAct->setEnabled(_grabber.isStreaming());
	_recordstartact->setEnabled(_grabber.isStreaming());

	if (!_grabber.isDeviceValid())
	{
		_TriggerModeAct->setEnabled(false);
		_TriggerModeAct->setChecked(false);
	}
	else
	{
		auto propmap = _grabber.devicePropertyMap();

		ic4::Error err;
		bool enabled = propmap.find(ic4::PropId::TriggerMode, err).getValue(err) == "On";
		if (err.isError())
		{
			enabled = propmap.findBoolean("Trigger", err).getValue(err);
		}

		if (err.isError())
		{
			_TriggerModeAct->setEnabled(false);
			_TriggerModeAct->setChecked(false);
		}
		else
		{
			_TriggerModeAct->setEnabled(true);
			_TriggerModeAct->setChecked(enabled);
		}
	}
}

void MainWindow::updateCameraLabel()
{
	ic4::Error err;
	auto deviceInfo = _grabber.deviceInfo(err);
	if (err.isSuccess())
	{
		auto text = deviceInfo.modelName() + " " + deviceInfo.serial();
		_sbCameralabel->setText(text.c_str());
	}
	else
	{
		_sbCameralabel->setText("No Device");
	}
}

/// <summary>
/// Show the device selection dialog for selecting a camera.
/// </summary>
void MainWindow::onSelectDevice()
{
	_grabber.deviceClose(ic4::Error::Ignore());

	DeviceSelectionDlg cDlg(this, &_grabber);
	if (cDlg.exec() == 1)
	{
		if (_grabber.isDeviceValid())
		{
			_grabber.deviceSaveState(_devicefile);
		}
		updateCameraLabel();
		startstopstream();
	}
	updateControls();
}

/// <summary>
/// Show the properties dialog of a device for e.g. exposure and gain adjustment
/// </summary>
void MainWindow::onDeviceProperties()
{
	PropertyMapDlg cDlg(_grabber.devicePropertyMap(), this);
	if (cDlg.exec() == 1)
	{
		_grabber.deviceSaveState(_devicefile);
	}

	updateControls();
}

void MainWindow::onToggleTriggerMode()
{
	auto propmap = _grabber.devicePropertyMap();

	if (!propmap.setValue(ic4::PropId::TriggerMode, _TriggerModeAct->isChecked(), ic4::Error::Ignore()))
	{
		propmap.setValue("Trigger", _TriggerModeAct->isChecked(), ic4::Error::Ignore());
	}
}


void MainWindow::startstopstream()
{
	try
	{
		if (_grabber.isDeviceValid())
		{
			if (_grabber.isStreaming())
			{
				_grabber.streamStop();
				if (_capturetovideo)
				{
					onStopCaptureVideo();
				}
			}
			else
			{
				_grabber.streamSetup(_queuesink, _display);
			}
		}
	}
	catch (ic4::IC4Exception ex)
	{
		QMessageBox::warning(NULL, _name.c_str(), ex.what());
	}

	updateControls();
}

/// <summary>
/// Handle the device lost event. Close video recording and
/// update the menu and toolbar.
/// </summary>

void MainWindow::onDeviceLost()
{
	QMessageBox::warning(NULL, _name.c_str(), "The video capture device is lost!");
	if (_capturetovideo)
	{
		onStopCaptureVideo();
	}
	updateCameraLabel();
	updateControls();
}

/// <summary>
//
/// </summary>
void MainWindow::onShootPhoto()
{
	std::lock_guard<std::mutex> guard(_snapphotomutex);
	_shootPhoto = true;
}

void MainWindow::savePhoto(const ic4::ImageBuffer& imagebuffer)
{
	static const QStringList filters(
	{
		"Bitmap(*.bmp)",
		"JPEG (*.jpg)",
		"Portable Network Graphics (*.png)",
		"TIFF (*.tif)"
	});

	QFileDialog dialog(this, tr("Save photo"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

	if (dialog.exec())
	{
#ifdef WIN32
		auto fileName = dialog.selectedFiles()[0].toStdWString();
#else
		auto fileName = dialog.selectedFiles()[0].toStdString();
#endif
		auto selectedFilter = dialog.selectedNameFilter();

		ic4::Error err;
		if (selectedFilter == filters[0])
			ic4::imageBufferSaveAsBitmap(imagebuffer, fileName, {}, err);
		else if (selectedFilter == filters[1])
			ic4::imageBufferSaveAsJpeg(imagebuffer, fileName, {}, err);
		else if (selectedFilter == filters[2])
			ic4::imageBufferSaveAsPng(imagebuffer, fileName, {}, err);
		else
			ic4::imageBufferSaveAsTiff(imagebuffer, fileName, {}, err);

		if (err.isError())
		{
			QMessageBox::warning(NULL, _name.c_str(), err.message().c_str());
		}
	}
}

void MainWindow::onStartCaptureVideo()
{
	const QStringList filters({ "MP4(*.mp4)" });

	QFileDialog dialog(this, tr("Capture Video"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));

	if (dialog.exec())
	{
#ifdef WIN32
		auto fileName = dialog.selectedFiles()[0].toStdWString();
#else
		auto fileName = dialog.selectedFiles()[0].toStdString();
#endif

		double fps = 25.0;
		auto propmap = _grabber.devicePropertyMap();
		try
		{
			fps = propmap.find(ic4::PropId::AcquisitionFrameRate).getValue();
			ic4::ImageType imgtype = _queuesink->getOutputImageType();
			_videowriter.beginFile(fileName, imgtype, fps);

			_capturetovideo = true;
			_videocapturepause = _recordpauseact->isChecked();
			_recordstopact->setEnabled(true);
		}
		catch (const ic4::IC4Exception& iex)
		{
			_videowriter.finishFile();
			_capturetovideo = false;
			_videocapturepause = _recordpauseact->isChecked();
			_recordstopact->setEnabled(false);
			_recordstartact->setChecked(false);

			QMessageBox::critical(this, _name.c_str(), iex.what());
		}
	}
	else
	{
		_recordstartact->setChecked(false);
		_recordstopact->setEnabled(false);
	}
}

void MainWindow::onPauseCaptureVideo()
{
	_videocapturepause = _recordpauseact->isChecked();
}

void MainWindow::onStopCaptureVideo()
{
	_capturetovideo = false;
	_videowriter.finishFile();
	_recordstartact->setChecked(false);
	_recordstopact->setEnabled(false);
}

void MainWindow::onCodecProperties()
{
	PropertyMapDlg cDlg(_videowriter.getPropertyMap(), this);
	if (cDlg.exec() == 1)
	{
		_videowriter.getPropertyMap().serialize(_codecconfigfile);
	}
}

/// <summary>
/// Listener related. Allocate image buffers.
/// </summary>
/// <param name="sink"></param>
/// <param name="imageType"></param>
/// <param name="min_buffers_required"></param>
/// <returns></returns>
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
	auto buffer = sink.popOutputBuffer();
	std::lock_guard<std::mutex> guard(_snapphotomutex);
	if (_shootPhoto)
	{
		_shootPhoto = false;
		// Send an event to the main thread with a reference to 
		// the main thread of our GUI. 
		QApplication::postEvent(this, new GotPhotoEvent(buffer));
	}

	if (_capturetovideo && !_videocapturepause)
	{
		// Save an image into our video file.
		_videowriter.addFrame(buffer);
	}
}


