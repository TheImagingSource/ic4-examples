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
#include "settings.h"
#include "fpscounter.h"

#include <filesystem>

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

		bool start_full_screen = false;
	};
		
	explicit MainWindow(const init_options& params, QWidget* parent = nullptr);

	~MainWindow();

	void showVideoFullScreen();

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
	void onDisplayContextMenu(const QPoint& pos);
	void onToggleFullScreen();
	void onFullScreenShowMenuBarToggled(bool checked);
	void onFullScreenShowToolBarToggled(bool checked);
	void onFullScreenShowStatusBarToggled(bool checked);
	void onExitFullScreen();
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
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	void customEvent(QEvent* event);
	void savePhoto(const ic4::ImageBuffer& imagebuffer);

	void prepareNewDeviceOpen();


	Settings _settings = {};

	std::filesystem::path _devicefile;		// File name of device state xml
	std::filesystem::path _codecconfigfile;   // File name of device state xml

	std::mutex _snapphotomutex;
	bool _shootPhoto = false;
	std::atomic<bool> _capturetovideo = false;
	std::atomic<bool> _videocapturepause = false;

	QGridLayout* mainLayout = nullptr;
	ic4interop::Qt::DisplayWidget* _VideoWidget = nullptr;

	QToolBar* _toolBar = nullptr;

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
	QAction* _toggleFullscreenAct = nullptr;
	QAction* _toggle_visibility_full_screen_menu_bar = nullptr;
	QAction* _toggle_visibility_full_screen_tool_bar = nullptr;
	QAction* _toggle_visibility_full_screen_status_bar = nullptr;

	QAction* _closeDeviceAct = nullptr;

	QLabel* _sbStatisticsLabel = nullptr;
	QLabel* _sbFpsLabel = nullptr;
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
	bool _preFullscreenMaximized = false;

	ic4demoapp::FpsCounter _fpsCounter;
};

#endif // MAINWINDOW_H
