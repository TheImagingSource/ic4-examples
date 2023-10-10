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

#include <string>
#include <iostream>
#include "utils.h"
#include "events.h"

#include "ic4dialogs/deviceselection/deviceselection.h"
#include "ic4dialogs/propertydlg/propertydlg.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	_display(nullptr),
	_queuesink(nullptr),
	_name("IC4 Demo App"),
	_shootPhoto(false),
	_videowriter(ic4::VideoWriterType::MP4_H264),
	_capturetovideo(false),
	_videocapturepause(false)
{
	this->setWindowTitle(_name.c_str());
	createUI();

	_devicefile = getAppDataDir(_name) + "/device.json";
	_codecconfigfile = getAppDataDir(_name) + "/codecconfig.json";

	_queuesink = ic4::QueueSink::create(*this);

	// Add a device lost handler. 
	_grabber.eventAddDeviceLost([this](ic4::Grabber& g) -> void {
		QApplication::postEvent(this, new QEvent(DEVICE_LOST_EVENT));
		});


	// Create the display for the live video and the sink for accessing images. 
	WId wid = _VideoWidget->winId();
	try
	{
		_display = ic4::Display::create(ic4::DisplayType::Default, (ic4::WindowHandle)wid);
		_display->setRenderPosition(ic4::DisplayRenderPosition::StretchCenter);
	}
	catch (ic4::IC4Exception ex)
	{
		QMessageBox::information(NULL, _name.c_str(), ex.what());
	}

	if (fileexist(_devicefile))
	{
		try
		{

			// Try to load the last used device.
			_grabber.deviceOpenFromState(_devicefile);

			_sbCameralabel->setText(std::string(_grabber.deviceInfo().modelName() + " " +
				_grabber.deviceInfo().serial()).c_str());
		}
		catch (...)
		{
			QMessageBox::information(NULL, _name.c_str(), "Loading last used device failed.");
		}
	}


	if (fileexist(_codecconfigfile))
	{
		try
		{
			_videowriter.getPropertyMap().deSerialize(_codecconfigfile);
		}
		catch (...)
		{
			QMessageBox::information(NULL, _name.c_str(), "Loading last codec configuration failed.");
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
	_window = new QWidget(this);
	setCentralWidget(_window);
	mainLayout = new QGridLayout;
	_window->setLayout(mainLayout);

	QToolBar* toolbar = new QToolBar(this);
	addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);

	auto fileMenu = menuBar()->addMenu(tr("&File"));

	auto exitAct = new QAction(tr("&Exit"), this);
	exitAct->setShortcuts(QKeySequence::Close);
	exitAct->setStatusTip(tr("Exit program"));
	connect(exitAct, &QAction::triggered, this, &QWidget::close);
	fileMenu->addAction(exitAct);

	////////////////////////////////////////////////////////////////////////////
	// Create the Device menu
	auto deviceMenu = menuBar()->addMenu(tr("&Device"));

	// Device Selection
	_DeviceSelectAct = new QAction(QIcon(":/images/camera.png"), tr("&Select"), this);
	_DeviceSelectAct->setStatusTip(tr("Select a video capture device"));
	connect(_DeviceSelectAct, &QAction::triggered, this, &MainWindow::onSelectDevice);
	QToolButton* btnDeviceSelection = new QToolButton();
	toolbar->addWidget(btnDeviceSelection);
	btnDeviceSelection->setDefaultAction(_DeviceSelectAct);
	deviceMenu->addAction(_DeviceSelectAct);

	////////////////////////////////////////////////////////////////////////////
	// Device Properites menu
	_DevicePropertiesAct = new QAction(QIcon(":/images/imgset.png"), tr("&Properties"), this);
	_DevicePropertiesAct->setStatusTip(tr("Show device property dialog"));
	connect(_DevicePropertiesAct, &QAction::triggered, this, &MainWindow::onDeviceProperties);
	QToolButton* btnDeviceProperties = new QToolButton();
	toolbar->addWidget(btnDeviceProperties);
	btnDeviceProperties->setDefaultAction(_DevicePropertiesAct);
	deviceMenu->addAction(_DevicePropertiesAct);
	_DevicePropertiesAct->setEnabled(false);

	toolbar->addSeparator();

	_TriggerModeAct = new QAction(QIcon(":/images/triggermode.png"), tr("&Trigger mode"), this);
	_TriggerModeAct->setStatusTip(tr("Enable and disable trigger mode"));
	_TriggerModeAct->setCheckable(true);
	connect(_TriggerModeAct, &QAction::triggered, this, &MainWindow::onToggleTriggerMode);
	auto btnTriggermode = new QToolButton();
	toolbar->addWidget(btnTriggermode);
	btnTriggermode->setDefaultAction(_TriggerModeAct);
	deviceMenu->addAction(_TriggerModeAct);
	_TriggerModeAct->setEnabled(false);
	_TriggerModeAct->setChecked(false);

	toolbar->addSeparator();

	_StartLiveAct = new QAction(QIcon(":/images/livestream.png"), tr("&Live stream"), this);
	_StartLiveAct->setStatusTip(tr("Start and stop the live stream"));
	_StartLiveAct->setCheckable(true);
	connect(_StartLiveAct, &QAction::triggered, this, &MainWindow::startstopstream);
	QToolButton* btnStartLive = new QToolButton();
	toolbar->addWidget(btnStartLive);
	btnStartLive->setDefaultAction(_StartLiveAct);
	deviceMenu->addAction(_StartLiveAct);
	_StartLiveAct->setEnabled(false);
	_StartLiveAct->setChecked(false);

	toolbar->addSeparator();

	////////////////////////////////////////////////////////////////////////////
	// capture menu
	auto captureMenu = menuBar()->addMenu(tr("&Capture"));

	_ShootPhotoAct = new QAction(QIcon(":/images/photo.png"), tr("&Shoot photo"), this);
	_ShootPhotoAct->setStatusTip(tr("Shoot and save a photo"));
	connect(_ShootPhotoAct, &QAction::triggered, this, &MainWindow::onShootPhoto);
	QToolButton* btnShootPhoto = new QToolButton();
	toolbar->addWidget(btnShootPhoto);
	btnShootPhoto->setDefaultAction(_ShootPhotoAct);
	captureMenu->addAction(_ShootPhotoAct);
	_ShootPhotoAct->setEnabled(false);

	toolbar->addSeparator();

	_recordstartact = new QAction(QIcon(":/images/recordstart.png"), tr("&Capture video"), this);
	_recordstartact->setStatusTip(tr("Capture video into MP4 file"));
	_recordstartact->setCheckable(true);
	connect(_recordstartact, &QAction::triggered, this, &MainWindow::onStartCaptureVideo);
	QToolButton* btnCaptureVideo = new QToolButton();
	toolbar->addWidget(btnCaptureVideo);
	btnCaptureVideo->setDefaultAction(_recordstartact);
	_recordstartact->setEnabled(false);
	_recordstartact->setChecked(false);
	captureMenu->addAction(_recordstartact);


	_recordpauseact = new QAction(QIcon(":/images/recordpause.png"), tr("&Pause capture video"), this);
	_recordpauseact->setStatusTip(tr("Pause video capture"));
	_recordpauseact->setCheckable(true);
	connect(_recordpauseact, &QAction::triggered, this, &MainWindow::onPauseCaptureVideo);
	btnCaptureVideo = new QToolButton();
	toolbar->addWidget(btnCaptureVideo);
	btnCaptureVideo->setDefaultAction(_recordpauseact);
	_recordpauseact->setEnabled(true);
	_recordpauseact->setChecked(false);
	captureMenu->addAction(_recordpauseact);

	_recordstopact = new QAction(QIcon(":/images/recordstop.png"), tr("&Stop capture video"), this);
	_recordstopact->setStatusTip(tr("End capture video into MP4 file"));
	_recordstopact->setCheckable(false);
	connect(_recordstopact, &QAction::triggered, this, &MainWindow::onStopCaptureVideo);
	btnCaptureVideo = new QToolButton();
	toolbar->addWidget(btnCaptureVideo);
	btnCaptureVideo->setDefaultAction(_recordstopact);
	_recordstopact->setEnabled(false);
	captureMenu->addAction(_recordstopact);


	_codecpropertyact = new QAction(QIcon(":/images/gear.png"), tr("&Codec properties"), this);
	_codecpropertyact->setStatusTip(tr("Configure the video codec"));
	_codecpropertyact->setCheckable(false);
	connect(_codecpropertyact, &QAction::triggered, this, &MainWindow::onCodecProperties);
	btnCaptureVideo = new QToolButton();
	toolbar->addWidget(btnCaptureVideo);
	btnCaptureVideo->setDefaultAction(_codecpropertyact);
	_codecpropertyact->setEnabled(true);

	captureMenu->addAction(_codecpropertyact);

	////////////////////////////////////////////////////////////////////////////
	// Create the video display Widget
	_VideoWidget = new QWidget();
	_VideoWidget->setMinimumSize(640, 480);
	mainLayout->addWidget(_VideoWidget);

	statusBar()->showMessage(tr("Ready"));
	_sbCameralabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(_sbCameralabel);
	_sbCameralabel->setText("No Device");
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
		_sbCameralabel->setText("No Device");

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

/// <summary>
/// Show the device selection dialog for selecting a camera.
/// </summary>
void MainWindow::onSelectDevice()
{
	if (_grabber.isStreaming())
	{
		_grabber.streamStop();
	}

	DeviceSelectionDlg cDlg(this, &_grabber);
	if (cDlg.exec() == 1)
	{
		if (_grabber.isDeviceValid())
		{
			_grabber.deviceSaveState(_devicefile);
			_sbCameralabel->setText(std::string(_grabber.deviceInfo().modelName() + " " + _grabber.deviceInfo().serial()).c_str());
		}
		else
		{
			_sbCameralabel->setText("No Device");
		}
	}
	updateControls();
}

/// <summary>
/// Show the properties dialog of a device for e.g. exposure and gain adjustment
/// </summary>
void MainWindow::onDeviceProperties()
{
	//PropertyMapDlg cDlg(_grabber.driverPropertyMap(), this);
	PropertyMapDlg cDlg(_grabber.devicePropertyMap(), this);
	if (cDlg.exec() == 1)
	{
		_grabber.deviceSaveState(_devicefile);
	}

	updateControls();
}

void MainWindow::onToggleTriggerMode()
{
	try
	{
		auto propmap = _grabber.devicePropertyMap();
		try
		{
			propmap.findBoolean("Trigger").setValue(_TriggerModeAct->isChecked());
		}
		catch (...)
		{
			propmap[ic4::PropId::TriggerMode].setValue(_TriggerModeAct->isChecked() ? "On" : "Off");
		}
	}
	catch (...)
	{
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
	dialog.setDirectory(getPictureDir(_name).c_str());

	if (dialog.exec())
	{
		auto fileName = dialog.selectedFiles()[0].toStdString();
		auto selectedFilter = dialog.selectedNameFilter().toStdString();

		ic4::Error err;
		if (selectedFilter == "Bitmap(*.bmp)")
			ic4::imageBufferSaveAsBitmap(imagebuffer, fileName, {}, err);
		else if (selectedFilter == "JPEG (*.jpg)")
			ic4::imageBufferSaveAsJpeg(imagebuffer, fileName, {}, err);
		else if (selectedFilter == "Portable Network Graphics (*.png)")
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
	dialog.setDirectory(getVideoDir(_name).c_str());

	if (dialog.exec())
	{
		auto fileName = dialog.selectedFiles()[0].toStdString();
		double fps = 25.0;
		auto propmap = _grabber.devicePropertyMap();
		try
		{
			fps = propmap.findFloat("AcquisitionFrameRate").getValue();
			ic4::ImageType imgtype;
			imgtype = _queuesink->getOutputImageType();
			_videowriter.beginFile(fileName.c_str(), imgtype, fps);

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
bool MainWindow::sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t 	min_buffers_required)
{
	sink.allocAndQueueBuffers(min_buffers_required + 2);
	return true;
};

/// <summary>
/// Listener related: Callbak for new frames. 
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


