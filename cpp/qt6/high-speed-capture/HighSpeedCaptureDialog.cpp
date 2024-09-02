
#include "HighSpeedCaptureDialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QByteArray>
#include <QThread>
#include <QtConcurrent>

#include <ic4-interop/interop-Qt.h>

#include "DeviceSelectionDialog.h"
#include "PropertyDialog.h"

const QEvent::Type UPDATE_STATS = static_cast<QEvent::Type>(QEvent::User + 1);

HighSpeedCaptureDialog::HighSpeedCaptureDialog()
{
	createUI();
	readSettings();
	updateUI();
}

void HighSpeedCaptureDialog::createUI()
{
	auto* layout = new QVBoxLayout();

	auto* buttonsLayout = new QHBoxLayout();

	_selectDevice = new QPushButton(tr("&Device"));
	connect(_selectDevice, &QPushButton::clicked, this, &HighSpeedCaptureDialog::onSelectDevice);
	buttonsLayout->addWidget(_selectDevice);

	_deviceProperties = new QPushButton(tr("Device &Properties"));
	connect(_deviceProperties, &QPushButton::clicked, this, &HighSpeedCaptureDialog::onDeviceProperties);
	buttonsLayout->addWidget(_deviceProperties);

	buttonsLayout->addStretch();

	layout->addLayout(buttonsLayout);

	auto* displayWidget = new ic4interop::Qt::DisplayWidget();
	displayWidget->setMinimumSize(640, 480);
	_display = displayWidget->asDisplay();
	_display->setRenderPosition(ic4::DisplayRenderPosition::StretchCenter);
	layout->addWidget(displayWidget, 1);

	auto* saveGroup = new QGroupBox(tr("Save Images"));
	auto* saveLayout = new QGridLayout();

	saveLayout->addWidget(new QLabel(tr("Destination Folder")), 0, 0);
	_destinationDirectory = new QLineEdit();
	_destinationDirectory->setReadOnly(true);
	saveLayout->addWidget(_destinationDirectory, 0, 1);
	_destinationBrowse = new QPushButton(tr("Browse"));
	saveLayout->addWidget(_destinationBrowse, 0, 2);

	connect(_destinationBrowse, &QPushButton::clicked,
		[this]()
		{
			auto dir = QFileDialog::getExistingDirectory(this, tr("Select Destination Directory"));
			if (!dir.isEmpty())
			{
				_destinationDirectory->setText(dir);
			}
		}
	);

	saveLayout->addWidget(new QLabel(tr("Buffer Memory")), 1, 0);

	auto* bufferMemoryLayout = new QHBoxLayout();

	_bufferMemory = new QLineEdit("4096");
	_bufferMemory->setMaximumWidth(120);
	_bufferMemory->setValidator(new QIntValidator());
	bufferMemoryLayout->addWidget(_bufferMemory);
	bufferMemoryLayout->addWidget(new QLabel(tr("MiB")));
	saveLayout->addLayout(bufferMemoryLayout, 1, 1);

	saveLayout->addWidget(new QLabel(tr("Free Buffers")), 2, 0);
	_freeBuffersProgress = new QProgressBar();
	saveLayout->addWidget(_freeBuffersProgress, 2, 1);
	_freeBuffersLabel = new QLabel();
	saveLayout->addWidget(_freeBuffersLabel, 2, 2);

	saveLayout->addWidget(new QLabel(tr("Filled Buffers")), 3, 0);
	_filledBuffersProgress = new QProgressBar();
	saveLayout->addWidget(_filledBuffersProgress, 3, 1);
	_filledBuffersLabel = new QLabel();
	saveLayout->addWidget(_filledBuffersLabel, 3, 2);

	_startStop = new QPushButton(tr("&Start"));
	connect(_startStop, &QPushButton::clicked, this, &HighSpeedCaptureDialog::onStartStop);
	saveLayout->addWidget(_startStop, 4, 0);
	_captureInfo = new QLabel();
	saveLayout->addWidget(_captureInfo, 4, 1, 1, 2);

	saveGroup->setLayout(saveLayout);
	layout->addWidget(saveGroup);

	setLayout(layout);
}

void HighSpeedCaptureDialog::updateUI()
{
	if (_sink != nullptr) // Presence of sink indicates capture is active
	{
		_startStop->setText(tr("&Stop"));
		_selectDevice->setEnabled(false);
		_bufferMemory->setEnabled(false);
		_destinationBrowse->setEnabled(false);
	}
	else
	{
		_startStop->setText(tr("&Start"));
		_selectDevice->setEnabled(true);
		_bufferMemory->setEnabled(true);
		_destinationBrowse->setEnabled(true);
	}

	if (_grabber.isDeviceValid())
	{
		_deviceProperties->setEnabled(true);
		_startStop->setEnabled(true);
	}
	else
	{
		_deviceProperties->setEnabled(false);
		_startStop->setEnabled(false);
	}
}

void HighSpeedCaptureDialog::readSettings()
{
	QSettings settings("The Imaging Source", "HighSpeedCapture Sample Application");

	auto defaultDestination = QString("%1/HighSpeedCapture Sample").arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	auto destinationDirectory = settings.value("DestinationDirectory", defaultDestination).toString();
	_destinationDirectory->setText(destinationDirectory);

	auto bufferMemory = settings.value("BufferMemory", "4096").toString();
	_bufferMemory->setText(bufferMemory);

	auto stateArray = settings.value("Device", QByteArray()).toByteArray();
	if (!stateArray.isEmpty())
	{
		std::vector<uint8_t> deviceState(stateArray.begin(), stateArray.end());

		if (_grabber.deviceOpenFromState(deviceState, ic4::Error::Ignore()))
		{
			_grabber.streamSetup(_display);
		}
	}
}

void HighSpeedCaptureDialog::saveSettings()
{
	QSettings settings("The Imaging Source", "HighSpeedCapture Sample Application");

	settings.setValue("DestinationDirectory", _destinationDirectory->text());
	settings.setValue("BufferMemory", _bufferMemory->text());

	auto deviceState = _grabber.deviceSaveState(ic4::Error::Ignore());
	if (!deviceState.empty())
	{
		QByteArray stateArray((const char*)deviceState.data(), deviceState.size());

		settings.setValue("Device", stateArray);
	}
}

void HighSpeedCaptureDialog::onSelectDevice()
{
	_grabber.streamStop();

	DeviceSelectionDialog dlg(this, &_grabber);
	dlg.exec();

	if (_grabber.isDeviceValid())
	{
		_grabber.streamSetup(_display);
	}

	updateUI();
}

void HighSpeedCaptureDialog::onDeviceProperties()
{
	if (_grabber.isDeviceValid())
	{
		if (_sink != nullptr)  // Presence of sink indicates capture is active
		{
			// Pass property map so that the dialog cannot restart the stream
			PropertyDialog dlg(_grabber.devicePropertyMap(), this);
			dlg.exec();
		}
		else
		{
			// Pass grabber itself so that the dialog can restart the stream
			PropertyDialog dlg(_grabber, this);
			dlg.exec();
		}
	}
}

void HighSpeedCaptureDialog::onStartStop()
{
	if (_sink == nullptr)
	{
		if (!QDir().mkpath(_destinationDirectory->text()))
		{
			QMessageBox msgError;
			msgError.setText("Failed to create destination directory");
			msgError.setIcon(QMessageBox::Critical);
			msgError.exec();
			return;
		}

		_grabber.streamStop();

		_sink = ic4::QueueSink::create(*this);
		_num_processed = 0;
		_frame_number = 0;

		_grabber.streamSetup(_sink, _display);

		updateUI();
	}
	else
	{
		_startStop->setEnabled(false);
		QApplication::setOverrideCursor(Qt::WaitCursor);

		// Stop the device
		_grabber.acquisitionStop();

		// Make sure we don't close the window while the cleanup thread runs
		_cleanup_active = true;

		// Collect and process remaining delivered buffers on background thread to keep UI responsive
		QtConcurrent::run(
			[this]()
			{
				while (!_cancel_cleanup)
				{
					{
						std::lock_guard lck(_frames_queued_mtx);

						// Wait for the sink's output queue to be emptied by repeated framesQueued invocations
						auto qs = _sink->queueSizes();
						if (qs.output_queue_length == 0)
							break;
					}

					QThread::usleep(1);
				}

				_cleanup_active = false;
			}
		).then(this, // Get back to the main thread to update UI
			[this]()
			{
				QApplication::restoreOverrideCursor();

				_grabber.streamStop();

				_sink = nullptr;

				_grabber.streamSetup(_display);

				updateUI();
			}
		);
	}	
}

void HighSpeedCaptureDialog::customEvent(QEvent* event)
{
	if (event->type() == UPDATE_STATS)
	{
		_filledBuffersProgress->setMaximum(_num_total);
		_filledBuffersProgress->setValue(_num_filled);
		_filledBuffersLabel->setText(QString("%1 / %2").arg(_num_filled).arg(_num_total));
		_freeBuffersProgress->setMaximum(_num_total);
		_freeBuffersProgress->setValue(_num_free);
		_freeBuffersLabel->setText(QString("%1 / %2").arg(_num_free).arg(_num_total));

		auto stats = _grabber.streamStatistics();
		auto numDropped = stats.device_transmission_error + stats.device_underrun + stats.sink_ignored + stats.sink_underrun;

		_captureInfo->setText(QString("Saved Images: %1 Frames Dropped: %2").arg(_num_processed).arg(numDropped));

		event->accept();
	}
}

void HighSpeedCaptureDialog::closeEvent(QCloseEvent* event)
{
	// Cancel and wait for possible cleanup thread
	_cancel_cleanup = true;
	while (_cleanup_active)
	{
		QThread::usleep(1);
	}

	// Make sure the stream is stopped so that framesQueued is no longer running
	_grabber.streamStop();

	saveSettings();
	event->accept();
}

bool HighSpeedCaptureDialog::sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required)
{
	auto bpp = ic4::getBitsPerPixel(imageType.pixel_format());
	auto imageSize = imageType.width() * imageType.height() * bpp / 8;

	auto numBuffers = _bufferMemory->text().toInt() * 1024ll * 1024ll / imageSize;

	if (numBuffers > min_buffers_required)
	{
		_num_total = numBuffers;
		sink.allocAndQueueBuffers(numBuffers);
	}
	else
	{
		_num_total = min_buffers_required;
	}	

	return true;
}

void HighSpeedCaptureDialog::framesQueued(ic4::QueueSink& sink)
{
	std::lock_guard lck(_frames_queued_mtx);

	{
		auto buffer = sink.popOutputBuffer();

		auto filePath = QString("%1/image_%2.png").arg(_destinationDirectory->text()).arg(_frame_number++);

		ic4::imageBufferSaveAsPng(*buffer, filePath.toStdString());

		_num_processed += 1;
	}

	auto queueSizes = _sink->queueSizes();
	_num_free = queueSizes.free_queue_length;
	_num_filled = queueSizes.output_queue_length;

	QCoreApplication::postEvent(this, new QEvent(UPDATE_STATS));
}