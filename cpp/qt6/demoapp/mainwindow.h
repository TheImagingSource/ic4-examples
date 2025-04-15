#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QtGui>
#include <QAction>
#include <QLabel>

#include <mutex>
#include <atomic>

#include <ic4/ic4.h> 
#include <ic4-interop/interop-Qt.h>

#include "PropertyDialog.h"

#include <filesystem>

namespace ic4demoapp
{
	inline auto QString_to_fspath(const QString& str) -> std::filesystem::path {
#if defined _WIN32
		return str.toStdWString();
#else
		return str.toStdString();
#endif
	}

	inline auto fspath_to_QString(const std::filesystem::path& str) -> QString {
#if defined _WIN32
		return QString::fromStdWString(str.wstring());
#else
		return QString::fromStdString(str.string());
#endif
	}
}


class MainWindow : public QMainWindow, ic4::QueueSinkListener
{
	Q_OBJECT

public:
	struct init_options {
		std::filesystem::path appDataDirectory;
		
		// If empty, then appDataDirectory / "device.json" is used, if deviceSetupFile.value().empty() no device is opened
		std::optional<std::filesystem::path> deviceSetupFile;

		// Show the settings menu
		bool show_settings_menu = false;
	};
		
	explicit MainWindow(const init_options& params, QWidget* parent = nullptr);

	~MainWindow();

private:
	void onSelectDevice();
	void onDeviceProperties();
	void onDeviceDriverProperties();
	void onToggleTriggerMode();
	void startstopstream();
	void onShootPhoto();
	void onStartCaptureVideo();
	void onPauseCaptureVideo();
	void onStopCaptureVideo();
	void onCodecProperties();
	void onDeviceLost();
	void onExportDeviceSettings();
	void onImportDeviceSettings();
	void onCloseDevice();
	void onAbout();

	void onDeviceOpened();
	void updateControls();
	void updateTriggerControl();
	void updateCameraLabel();
	void updateStatistics();
	void onUpdateStatisticsTimer();

	void createUI();

protected:
	void closeEvent(QCloseEvent* ev) override;
	void changeEvent(QEvent* ev) override;

private:
	void customEvent(QEvent* event);
	void savePhoto(const ic4::ImageBuffer& imagebuffer);

	std::filesystem::path _devicefile;		// File name of device state xml
	std::filesystem::path _codecconfigfile;   // File name of device state xml

	std::mutex _snapphotomutex;
	bool _shootPhoto = false;
	std::atomic<bool> _capturetovideo = false;
	std::atomic<bool> _videocapturepause = false;

	QGridLayout* mainLayout = nullptr;
	ic4interop::Qt::DisplayWidget* _VideoWidget = nullptr;

	QAction* _DeviceSelectAct = nullptr;
	QAction* _DevicePropertiesAct = nullptr;
    QAction* _DeviceDriverPropertiesAct = nullptr;
	QAction* _TriggerModeAct = nullptr;
	QAction* _StartLiveAct = nullptr;
	QAction* _ShootPhotoAct = nullptr;
	QAction* _recordstartact = nullptr;
	QAction* _recordpauseact = nullptr;
	QAction* _recordstopact = nullptr;
	QAction* _codecpropertyact = nullptr;
	QAction* _exportDeviceSettingsAct = nullptr;
	QAction* _importDeviceSettingsAct = nullptr;

	QAction* _closeDeviceAct = nullptr;

	QLabel* _sbStatisticsLabel = nullptr;
	QLabel* _sbCameraLabel = nullptr;

	QTimer* _updateStatisticsTimer = nullptr;

	ic4::PropertyMap _devicePropertyMap;
	ic4::Grabber _grabber;
	std::shared_ptr<ic4::Display> _display;
	std::shared_ptr<ic4::QueueSink> _queuesink;
	ic4::VideoWriter _videowriter;

	PropertyDialog* _propertyDialog = nullptr;

    bool sinkConnected( ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required ) final;
    void framesQueued( ic4::QueueSink& sink ) final;

	bool _showSettingsMenu = false;
	bool _start_stream_on_open = true;

	ic4::PropVisibility _defaultVisibility = ic4::PropVisibility::Beginner;

	void readSettingsFile(const std::filesystem::path& appDataDirectory);
};

#endif // MAINWINDOW_H
