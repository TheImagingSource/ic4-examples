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
#include "events.h"

#include "DeviceSelectionDialog.h"
#include "ResourceSelector.h"

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
#include <QClipboard>
#include <QTimer>
#include <QKeySequence>

#include <filesystem>
#include <string>

MainWindow::MainWindow(const init_options& params, QWidget* parent)
	: QMainWindow(parent)
	, _videowriter(ic4::VideoWriterType::MP4_H264)
{
	_showSettingsMenu = params.show_settings_menu;

	// Make sure the %appdata%/demoapp directory exists
	if (!params.appDataDirectory.empty())
	{
		QDir(QString::fromStdString(params.appDataDirectory.string())).mkpath(".");

		_devicefile = params.appDataDirectory / "device.json";
		_codecconfigfile = params.appDataDirectory / "codecconfig.json";
		readSettingsFile(params.appDataDirectory);
	}


	createUI();

	// Create the sink for accessing images.
	_queuesink = ic4::QueueSink::create(*this);

	// Add a device lost handler. 
	_grabber.eventAddDeviceLost([this](ic4::Grabber& g) {
		QApplication::postEvent(this, new QEvent(DEVICE_LOST_EVENT));
	});

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

	std::filesystem::path deviceSetupFile_value;
	if (!params.deviceSetupFile.has_value())
	{
		deviceSetupFile_value = _devicefile;
	}
	else
	{
		deviceSetupFile_value = params.deviceSetupFile.value();
	}

	if (std::filesystem::is_regular_file(deviceSetupFile_value))
	{
		ic4::Error err;

		// Try to load the last used device.
		if (!_grabber.deviceOpenFromState(deviceSetupFile_value, err))
		{
			auto message = "Loading last used device failed: " + err.message();
			QMessageBox::information(this, {}, message.c_str());
		}

		onDeviceOpened();
	}

	if (std::filesystem::is_regular_file(_codecconfigfile))
	{
		ic4::Error err;

		try
		{
			_videowriter.propertyMap().deSerialize(_codecconfigfile, err);
		}
		catch (const ic4::IC4Exception& ex)
		{
			auto message = "Loading last codec configuration failed: " + std::string(ex.what());
			QMessageBox::information(this, {}, message.c_str());
		}
	}

	updateControls();

	if (params.start_full_screen)
	{
		onToggleFullScreen();
	}
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

	////////////////////////////////////////////////////////////////////////////
	// Define program actions

	// Use our resource selector that will choose the appropriate icon file
	// depending on current darkmode setting
	ResourceSelector selector;

	// Device Selection
	_DeviceSelectAct = new QAction(selector.loadIcon(":/images/camera.png"), tr("&Select"), this);
	_DeviceSelectAct->setStatusTip(tr("Select a video capture device"));
	_DeviceSelectAct->setShortcut(QKeySequence::Open);
	connect(_DeviceSelectAct, &QAction::triggered, this, &MainWindow::onSelectDevice);

	// Device Properties
	_DevicePropertiesAct = new QAction(selector.loadIcon(":/images/imgset.png"), tr("&Properties"), this);
	_DevicePropertiesAct->setStatusTip(tr("Show device property dialog"));
	_DevicePropertiesAct->setEnabled(false);
	_DevicePropertiesAct->setShortcut(QKeySequence::Print);
	connect(_DevicePropertiesAct, &QAction::triggered, this, &MainWindow::onDeviceProperties);

    // Device Driver Properties
	_DeviceDriverPropertiesAct = new QAction(tr("&Driver Properties"), this);
	_DeviceDriverPropertiesAct->setStatusTip(tr("Show device driver property dialog"));
	_DeviceDriverPropertiesAct->setEnabled(false);
    connect(_DeviceDriverPropertiesAct, &QAction::triggered, this, &MainWindow::onDeviceDriverProperties);

	// Trigger Mode
	_TriggerModeAct = new QAction(selector.loadIcon(":/images/triggermode.png"), tr("&Trigger mode"), this);
	_TriggerModeAct->setStatusTip(tr("Enable and disable trigger mode"));
	_TriggerModeAct->setCheckable(true);
	_TriggerModeAct->setEnabled(false);
	_TriggerModeAct->setChecked(false);
	connect(_TriggerModeAct, &QAction::triggered, this, &MainWindow::onToggleTriggerMode);

	// Start/Stop Live Stream
	_StartLiveAct = new QAction(selector.loadIcon(":/images/livestream.png"), tr("&Live stream"), this);
	_StartLiveAct->setStatusTip(tr("Start and stop the live stream"));
	_StartLiveAct->setCheckable(true);
	_StartLiveAct->setEnabled(false);
	_StartLiveAct->setChecked(false);
	connect(_StartLiveAct, &QAction::triggered, this, &MainWindow::startstopstream);

	// Capture Photo
	_ShootPhotoAct = new QAction(selector.loadIcon(":/images/photo.png"), tr("&Shoot photo"), this);
	_ShootPhotoAct->setStatusTip(tr("Shoot and save a photo"));
	_ShootPhotoAct->setEnabled(false);
	_ShootPhotoAct->setShortcut(QKeySequence::Save);
	connect(_ShootPhotoAct, &QAction::triggered, this, &MainWindow::onShootPhoto);

	// Capture Video
	_recordstartact = new QAction(selector.loadIcon(":/images/recordstart.png"), tr("&Capture video"), this);
	_recordstartact->setStatusTip(tr("Capture video into MP4 file"));
	_recordstartact->setCheckable(true);
	connect(_recordstartact, &QAction::triggered, this, &MainWindow::onStartCaptureVideo);

	// Pause Video Recording
	_recordpauseact = new QAction(selector.loadIcon(":/images/recordpause.png"), tr("&Pause capture video"), this);
	_recordpauseact->setStatusTip(tr("Pause video capture"));
	_recordpauseact->setCheckable(true);
	_recordpauseact->setEnabled(true);
	_recordpauseact->setChecked(false);
	connect(_recordpauseact, &QAction::triggered, this, &MainWindow::onPauseCaptureVideo);

	// Stop Video Recording
	_recordstopact = new QAction(selector.loadIcon(":/images/recordstop.png"), tr("&Stop capture video"), this);
	_recordstopact->setStatusTip(tr("End capture video into MP4 file"));
	_recordstopact->setEnabled(false);
	connect(_recordstopact, &QAction::triggered, this, &MainWindow::onStopCaptureVideo);

	// Codec Properties
	_codecpropertyact = new QAction(selector.loadIcon(":/images/gear.png"), tr("&Codec properties"), this);
	_codecpropertyact->setStatusTip(tr("Configure the video codec"));
	connect(_codecpropertyact, &QAction::triggered, this, &MainWindow::onCodecProperties);

	// Export/Import device settings
	_exportDeviceSettingsAct = new QAction(tr("&Export Device Settings..."), this);
	_exportDeviceSettingsAct->setStatusTip(tr("Export the current device settings as a json file"));
	connect(_exportDeviceSettingsAct, &QAction::triggered, this, &MainWindow::onExportDeviceSettings);
	_importDeviceSettingsAct = new QAction(tr("&Import Device Settings..."), this);
	_importDeviceSettingsAct->setStatusTip(tr("Open a device with settings from a json file"));
	connect(_importDeviceSettingsAct, &QAction::triggered, this, &MainWindow::onImportDeviceSettings);

	// Close device
	_closeDeviceAct = new QAction(tr("&Close"), this);
	_closeDeviceAct->setStatusTip(tr("Close the currently opened device"));
	_closeDeviceAct->setShortcuts(QKeySequence::Close);
	connect(_closeDeviceAct, &QAction::triggered, this, &MainWindow::onCloseDevice);

	_toggleFullscreenAct = new QAction(tr("&Full Screen"), this);
	_toggleFullscreenAct->setStatusTip(tr("Toggle full screen display"));
	_toggleFullscreenAct->setShortcut(Qt::Key_F11);
	connect(_toggleFullscreenAct, &QAction::triggered, this, &MainWindow::onToggleFullScreen);

	auto exitFullscreenAct = new QAction(this);
	exitFullscreenAct->setShortcut(Qt::Key_Escape);
	connect(exitFullscreenAct, &QAction::triggered, this, &MainWindow::onExitFullScreen);

	auto aboutAct = new QAction(tr("&About..."), this);
	aboutAct->setStatusTip(tr("About ic4-demoapp"));
	connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);

	// Exit Program
	auto exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit program"));
	connect(exitAct, &QAction::triggered, this, &QWidget::close);

	// Settings Menu
	auto defaultVisibilityMenu = new QMenu(tr("Default &Visibility"), this);
	defaultVisibilityMenu->setStatusTip(tr("Sets the default visibility"));
	auto beginnerEntry = defaultVisibilityMenu->addAction(tr("Beginner"));
	beginnerEntry->setCheckable(true);
	beginnerEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Beginner);
	auto expertEntry = defaultVisibilityMenu->addAction(tr("Expert"));
	expertEntry->setCheckable(true);
	expertEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Expert);
	auto guruEntry = defaultVisibilityMenu->addAction(tr("Guru"));
	guruEntry->setCheckable(true);
	guruEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Guru);

	auto update_entries = [this, beginnerEntry, expertEntry, guruEntry] {
		beginnerEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Beginner);
		expertEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Expert);
		guruEntry->setChecked(_defaultVisibility == ic4::PropVisibility::Guru);
	};

	connect(beginnerEntry, &QAction::triggered, [this, update_entries] { _defaultVisibility = ic4::PropVisibility::Beginner; update_entries(); });
	connect(expertEntry, &QAction::triggered, [this, update_entries] { _defaultVisibility = ic4::PropVisibility::Expert; update_entries(); });
	connect(guruEntry, &QAction::triggered, [this, update_entries] { _defaultVisibility = ic4::PropVisibility::Guru; update_entries(); });

	auto deleteDeviceSettingsFile  = new QAction(tr("Delete Device Settings File"), this);
	deleteDeviceSettingsFile->setStatusTip(tr("Deletes the current device settings file"));
	connect(deleteDeviceSettingsFile, &QAction::triggered,
		[this] {
			if (!std::filesystem::is_regular_file(_devicefile)) {
				return;
			}
			std::error_code ec;
			std::filesystem::remove( _devicefile, ec );
			if (ec) {
				qWarning().noquote() << "Failed to delete: " << QString::fromStdString(_devicefile.string());
			}
		}
	);

	auto start_stream_on_open = new QAction(tr("Start stream on open"), this);
	start_stream_on_open->setCheckable(true);
	start_stream_on_open->setChecked(_start_stream_on_open);
	connect(start_stream_on_open, &QAction::triggered, [this, start_stream_on_open] { _start_stream_on_open = !_start_stream_on_open; start_stream_on_open->setChecked(_start_stream_on_open); });

	////////////////////////////////////////////////////////////////////////////
	// Create the File Menu
	auto fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(exitAct);
	////////////////////////////////////////////////////////////////////////////
	// Create the Device Menu
	auto deviceMenu = menuBar()->addMenu(tr("&Device"));
	deviceMenu->addAction(_DeviceSelectAct);
	deviceMenu->addAction(_DevicePropertiesAct);
    deviceMenu->addAction(_DeviceDriverPropertiesAct);
	deviceMenu->addAction(_TriggerModeAct);
	deviceMenu->addAction(_StartLiveAct);
	deviceMenu->addSeparator();
	deviceMenu->addAction(_exportDeviceSettingsAct);
	deviceMenu->addAction(_importDeviceSettingsAct);
	deviceMenu->addSeparator();
	deviceMenu->addAction(_closeDeviceAct);
	////////////////////////////////////////////////////////////////////////////
	// Create the Capture Menu
	auto captureMenu = menuBar()->addMenu(tr("&Capture"));
	captureMenu->addAction(_ShootPhotoAct);
	captureMenu->addAction(_recordstartact);
	captureMenu->addAction(_recordpauseact);
	captureMenu->addAction(_recordstopact);
	captureMenu->addAction(_codecpropertyact);

	////////////////////////////////////////////////////////////////////////////
	// Create the Settings Menu
	auto settingsMenu = menuBar()->addMenu(tr("&Settings"));
	settingsMenu->addMenu(defaultVisibilityMenu);
	settingsMenu->addAction(deleteDeviceSettingsFile);
	settingsMenu->addAction(start_stream_on_open);
	settingsMenu->menuAction()->setVisible(_showSettingsMenu);

	////////////////////////////////////////////////////////////////////////////
	// Create the View Menu
	auto viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(_toggleFullscreenAct);

	////////////////////////////////////////////////////////////////////////////
	// Create the Help Menu
	auto helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);

	////////////////////////////////////////////////////////////////////////////
	// Create the Toolbar
	QToolBar* toolbar = new QToolBar("&Toolbar", this);
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
	_VideoWidget = new ic4interop::Qt::DisplayWidget(this);
	_VideoWidget->setMinimumSize(640, 480);
	setCentralWidget(_VideoWidget);

	statusBar()->showMessage(tr("Ready"));
	_sbStatisticsLabel = new QLabel("");
	statusBar()->addPermanentWidget(_sbStatisticsLabel);
	statusBar()->addPermanentWidget(new QLabel("  "));
	_sbCameraLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(_sbCameraLabel);

	_updateStatisticsTimer = new QTimer(this);
	connect(_updateStatisticsTimer, &QTimer::timeout, this, &MainWindow::onUpdateStatisticsTimer);
	_updateStatisticsTimer->start(100);

	_VideoWidget->installEventFilter(this);
	_VideoWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(_VideoWidget, &QWidget::customContextMenuRequested, this, &MainWindow::onDisplayContextMenu);

	// Add commands with hot keys to the video widgets so that hot keys work in full screen mode
	_VideoWidget->addAction(_toggleFullscreenAct);
	_VideoWidget->addAction(_DeviceSelectAct);
	_VideoWidget->addAction(_closeDeviceAct);
	_VideoWidget->addAction(_DevicePropertiesAct);
	_VideoWidget->addAction(_exportDeviceSettingsAct);
	_VideoWidget->addAction(_importDeviceSettingsAct);
	_VideoWidget->addAction(exitFullscreenAct);
}

void MainWindow::closeEvent(QCloseEvent* ev)
{
	if (_grabber.isDeviceValid())
	{
		_grabber.deviceSaveState(_devicefile);
	}
}

void MainWindow::changeEvent(QEvent* ev)
{
	switch (ev->type())
	{
	case QEvent::ThemeChange:
		{
			// Re-configure action icons after theme change
			ResourceSelector selector;
			_DeviceSelectAct->setIcon(selector.loadIcon(":/images/camera.png"));
			_DevicePropertiesAct->setIcon(selector.loadIcon(":/images/imgset.png"));
			_TriggerModeAct->setIcon(selector.loadIcon(":/images/triggermode.png"));
			_StartLiveAct->setIcon(selector.loadIcon(":/images/livestream.png"));
			_ShootPhotoAct->setIcon(selector.loadIcon(":/images/photo.png"));
			_recordstartact->setIcon(selector.loadIcon(":/images/recordstart.png"));
			_recordpauseact->setIcon(selector.loadIcon(":/images/recordpause.png"));
			_recordstopact->setIcon(selector.loadIcon(":/images/recordstop.png"));
			_codecpropertyact->setIcon(selector.loadIcon(":/images/gear.png"));
			break;
		}
	case QEvent::ActivationChange:
		if (isActiveWindow())
		{
			onExitFullScreen();
		}
		break;
	default:
		break;
	}
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == _VideoWidget)
	{
		switch (event->type())
		{
		case QEvent::MouseButtonDblClick:
			if (((QMouseEvent*)event)->button() == Qt::MouseButton::LeftButton)
			{
				onToggleFullScreen();
				return true;
			}
			break;
		case QEvent::Close:
			// Forward Alt-F4 to main window
			close();
			break;
		}
	}

	return false;
}

void MainWindow::updateStatistics()
{
	ic4::Error err;
	auto stats = _grabber.streamStatistics(err);
	if (err.isSuccess())
	{
		auto text = QString("Frames Delivered: %1 Dropped: %2/%3/%4/%5")
			.arg(stats.sink_delivered)
			.arg(stats.device_transmission_error)
			.arg(stats.device_underrun)
			.arg(stats.transform_underrun)
			.arg(stats.sink_underrun);

		_sbStatisticsLabel->setText(text);

		auto tooltip = QString(
			"Frames Delivered: %1\n\n"
			"Frames Dropped:\n"
			"  Device Transmission Error: %2\n"
			"  Device Underrun: %3\n"
			"  Transform Underrun: %4\n"
			"  Sink Underrun: %5")
			.arg(stats.sink_delivered)
			.arg(stats.device_transmission_error)
			.arg(stats.device_underrun)
			.arg(stats.transform_underrun)
			.arg(stats.sink_underrun);

		_sbStatisticsLabel->setToolTip(tooltip);
	}
	else
	{
		qWarning().noquote() << "Failed query stream statistics:" << err.message().c_str();
	}
}

void MainWindow::onUpdateStatisticsTimer()
{
	if (!_grabber.isStreaming())
		return;

	updateStatistics();
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
	if (!_grabber.isDeviceOpen())
		_sbStatisticsLabel->clear();

	_DevicePropertiesAct->setEnabled(_grabber.isDeviceValid());
	_DeviceDriverPropertiesAct->setEnabled(_grabber.isDeviceValid());
	_exportDeviceSettingsAct->setEnabled(_grabber.isDeviceValid());
	_closeDeviceAct->setEnabled(_grabber.isDeviceOpen());
	_StartLiveAct->setEnabled(_grabber.isDeviceValid());
	_StartLiveAct->setChecked(_grabber.isStreaming());
	_ShootPhotoAct->setEnabled(_grabber.isStreaming());
	_recordstartact->setEnabled(_grabber.isStreaming());

	updateTriggerControl();
}

void MainWindow::updateTriggerControl()
{
	if (!_grabber.isDeviceValid())
	{
		_TriggerModeAct->setEnabled(false);
		_TriggerModeAct->setChecked(false);
	}
	else
	{
		ic4::Error err;
		bool enabled = _devicePropertyMap.getValueString(ic4::PropId::TriggerMode, err) == "On";

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
		_sbCameraLabel->setText(text.c_str());
	}
	else
	{
		_sbCameraLabel->setText("No Device");
	}
}

/// <summary>
/// Show the device selection dialog for selecting a camera.
/// </summary>
void MainWindow::onSelectDevice()
{	
	DeviceSelectionDialog cDlg(_VideoWidget, &_grabber);
	if (cDlg.exec() == 1)
	{
		if (_propertyDialog != nullptr)
		{
			_propertyDialog->updateGrabber(_grabber);
		}

		onDeviceOpened();
	}
	updateControls();
}

void MainWindow::onDeviceOpened()
{
	// Remember the device's property map for later use
	_devicePropertyMap = _grabber.devicePropertyMap(ic4::Error::Ignore());

	auto triggerMode = _devicePropertyMap.find(ic4::PropId::TriggerMode, ic4::Error::Ignore());
	triggerMode.eventAddNotification([this](ic4::Property&) { updateTriggerControl(); }, ic4::Error::Ignore());

	updateCameraLabel();
	if (_start_stream_on_open)
	{
		startstopstream();
	}
}

/// <summary>
/// Show the properties dialog of a device for e.g. exposure and gain adjustment
/// </summary>
void MainWindow::onDeviceProperties()
{
	if (_propertyDialog == nullptr)
	{
		_propertyDialog = new PropertyDialog(_grabber, _VideoWidget, tr("Device Properties"));
		_propertyDialog->setPropVisibility(_defaultVisibility);
	}

	_propertyDialog->show();
}

void MainWindow::onDeviceDriverProperties()
{
    PropertyDialog cDlg(_grabber.driverPropertyMap(), _VideoWidget, tr("Device Driver Properties"));
	cDlg.setPropVisibility(_defaultVisibility);
	
	cDlg.exec();

    updateControls();
}

void MainWindow::onToggleTriggerMode()
{
	ic4::Error err;
	if (!_devicePropertyMap.setValue(ic4::PropId::TriggerMode, _TriggerModeAct->isChecked(), err))
	{
		QMessageBox::critical(this, {}, QString::fromStdString(err.message()));
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

				// Update statistics one final time (auto update is disabled while stream is stopped)
				updateStatistics();
			}
			else
			{
				_grabber.streamSetup(_queuesink, _display);
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
/// Handle the device lost event. Close video recording and
/// update the menu and toolbar.
/// </summary>

void MainWindow::onDeviceLost()
{
	QMessageBox::warning(this, {}, "The video capture device is lost!");
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

	static auto savePictureDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

	QFileDialog dialog(_VideoWidget, tr("Save photo"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(savePictureDirectory);

	if (dialog.exec())
	{
		auto selectedFilter = dialog.selectedNameFilter();

		auto fullPath = dialog.selectedFiles()[0];
		savePictureDirectory = QFileInfo(fullPath).absolutePath();
#ifdef WIN32
		auto platformFileName = fullPath.toStdWString();
#else
		auto platformFileName = fullPath.toStdString();
#endif

		ic4::Error err;
		if (selectedFilter == filters[0])
			ic4::imageBufferSaveAsBitmap(imagebuffer, platformFileName, {}, err);
		else if (selectedFilter == filters[1])
			ic4::imageBufferSaveAsJpeg(imagebuffer, platformFileName, {}, err);
		else if (selectedFilter == filters[2])
			ic4::imageBufferSaveAsPng(imagebuffer, platformFileName, {}, err);
		else
			ic4::imageBufferSaveAsTiff(imagebuffer, platformFileName, {}, err);

		if (err.isError())
		{
			QMessageBox::critical(this, {}, err.message().c_str());
		}
	}
}

void MainWindow::onStartCaptureVideo()
{
	if (_capturetovideo)
	{
		onStopCaptureVideo();
		return;
	}

	const QStringList filters({ "MP4 Video Files (*.mp4)" });

	static auto saveVideoDirectory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

	QFileDialog dialog(_VideoWidget, tr("Capture Video"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(saveVideoDirectory);

	if (dialog.exec())
	{
		auto fullPath = dialog.selectedFiles()[0];
		saveVideoDirectory = QFileInfo(fullPath).absolutePath();
#ifdef WIN32
		auto platformFileName = fullPath.toStdWString();
#else
		auto platformFileName = fullPath.toStdString();
#endif

		double fps = 25.0;
		try
		{
			fps = _devicePropertyMap.getValueDouble(ic4::PropId::AcquisitionFrameRate);
			ic4::ImageType imgtype = _queuesink->outputImageType();
			_videowriter.beginFile(platformFileName, imgtype, fps);

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

			QMessageBox::critical(this, {}, iex.what());
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
	PropertyDialog cDlg(_videowriter.propertyMap(), _VideoWidget, tr("Codec Settings"));
	cDlg.setPropVisibility(_defaultVisibility);
	if (cDlg.exec() == 1)
	{
		_videowriter.propertyMap().serialize(_codecconfigfile);
	}
}

void MainWindow::onExportDeviceSettings()
{
	const QStringList filters({ "Device Settings Files (*.json)" });

	QFileDialog dialog(_VideoWidget, tr("Export Device Settings"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

	if (dialog.exec())
	{
#ifdef WIN32
		auto fileName = dialog.selectedFiles()[0].toStdWString();
#else
		auto fileName = dialog.selectedFiles()[0].toStdString();
#endif

		ic4::Error err;
		if (!_grabber.deviceSaveState(fileName, err))
		{
			QMessageBox::critical(this, {}, err.message().c_str());
		}
	}
}

void MainWindow::onImportDeviceSettings()
{
	const QStringList filters({ "Device Settings Files (*.json)", "All Files (*.*)"});

	QFileDialog dialog(_VideoWidget, tr("Import Device Settings"));
	dialog.setNameFilters(filters);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

	if (dialog.exec())
	{
#ifdef WIN32
		auto fileName = dialog.selectedFiles()[0].toStdWString();
#else
		auto fileName = dialog.selectedFiles()[0].toStdString();
#endif

		// Stop stream/recording if active
		if (_grabber.isStreaming())
		{
			startstopstream();
		}

		_grabber.deviceClose(ic4::Error::Ignore());

		ic4::Error err;
		if( !_grabber.deviceOpenFromState(fileName, err) )
		{
			QMessageBox::critical(this, {}, err.message().c_str());
		}
		else
		{
			// Remember the device's property map for later use
			_devicePropertyMap = _grabber.devicePropertyMap(ic4::Error::Ignore());

			// Restart stream
			if (this->_start_stream_on_open)
				startstopstream();
		}
	}
}

void MainWindow::onCloseDevice()
{
	if (_grabber.isStreaming())
	{
		startstopstream();
	}

	if (_capturetovideo)
	{
		_videowriter.finishFile(ic4::Error::Ignore());
	}

	_grabber.deviceClose(ic4::Error::Ignore());
	_devicePropertyMap = {};
	_display->displayBuffer(nullptr, ic4::Error::Ignore());

    updateCameraLabel();
	updateControls();
}

void MainWindow::onAbout()
{
	auto ver = QApplication::applicationVersion();
	if (ver.isEmpty()) {
		ver = "(No version info available)";
	} else {
		ver = "Version " + ver;
	}

#if defined _WIN32
#	if defined _M_ARM
	ver += " ARMv7";
#	elif defined _M_ARM64
	ver += " ARM64";
#	elif defined _M_ARM64EC
	ver += " ARM64EC";
#	elif defined _M_AMD64 
	ver += " x64";
#	endif
#endif

	QMessageBox about(this);
	about.setWindowTitle("About");
	about.setTextFormat(Qt::RichText);
	about.setIcon(QMessageBox::Information);
	about.setText(
		QString(
			"%1"
			"<br/><br/>"
			"%2"
			"<br/><br/>"
			"The source code for %1 is available as a C++ example program:"
			"<br/>"
			"<a href='%3'>%3</a>"
		)
		.arg(QApplication::applicationDisplayName())
		.arg(ver)
		.arg("https://github.com/TheImagingSource/ic4-examples")
	);
	about.exec();
}

void MainWindow::onDisplayContextMenu(const QPoint& pos)
{
	QMenu* menu = new QMenu(_VideoWidget);
	menu->addAction(_toggleFullscreenAct);
	menu->addSeparator();
	menu->addAction(_DeviceSelectAct);
	menu->addAction(_DevicePropertiesAct);
	menu->addSeparator();
	menu->addAction(_ShootPhotoAct);
	menu->addSeparator();
	menu->addAction(_exportDeviceSettingsAct);
	menu->addAction(_importDeviceSettingsAct);
	menu->popup(_VideoWidget->mapToGlobal(pos));
}

void MainWindow::onToggleFullScreen()
{
	if (_VideoWidget->isFullScreen())
	{
		setCentralWidget(_VideoWidget);

		_VideoWidget->showNormal();

		show();
	}
	else
	{
		hide();

		_VideoWidget->setParent(nullptr);
		// move the stand-alone widget to the position of the main window
		// parentless widgets default their position to 0:0
		// this causes the fullscreen widget to always appear in the upper left corner
		// we want to widget to be on the same monitor as the main window
		_VideoWidget->move(pos());
		_VideoWidget->showFullScreen();
	}
}

void MainWindow::onExitFullScreen()
{
	if( _VideoWidget->isFullScreen() )
	{
		onToggleFullScreen();
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
	ic4::Error err;
	auto buffer = sink.popOutputBuffer(err);
	if (!buffer)
	{		
		qWarning().noquote() << "Failed to query buffer from QueueSink:" << err.message().c_str();
		return;
	}

	// Connect the buffer's chunk data to the device's property map
	// This allows for properties backed by chunk data to be updated
	if (!_devicePropertyMap.connectChunkData(buffer, err))
	{
		qWarning().noquote() << "Failed to connect new buffer to the device's property map:" << err.message().c_str();

		_grabber.devicePropertyMap(ic4::Error::Ignore()).connectChunkData(nullptr, ic4::Error::Ignore());
	}
		
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
		if (!_videowriter.addFrame(buffer, err))
		{
			qWarning().noquote() << "Failed to add frame to video file:" << err.message().c_str();
		}
	}
}

void MainWindow::readSettingsFile(const std::filesystem::path& appDataDirectory)
{
	QFile file(ic4demoapp::fspath_to_QString(appDataDirectory / "settings.json") );
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return;
	}

	_showSettingsMenu = true;

	QString val = file.readAll();
	file.close();

	QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
	QJsonObject json_mainwindows = d.object().value("MainWindow").toObject();
	if (json_mainwindows.empty()) {
		return;
	}
	if (auto val = json_mainwindows["Default Visibility"]; val.isDouble()) {
		switch (val.toInt())
		{
		case 0:	_defaultVisibility = ic4::PropVisibility::Beginner;	break;
		case 1:	_defaultVisibility = ic4::PropVisibility::Expert; break;
		case 2:	_defaultVisibility = ic4::PropVisibility::Guru; break;
		case 3:	_defaultVisibility = ic4::PropVisibility::Invisible; break;
		default:
			break;
		}
	}
	if (auto val = json_mainwindows["Start stream on open"]; val.isBool()) {
		_start_stream_on_open = val.toBool(_start_stream_on_open);
	}

}

