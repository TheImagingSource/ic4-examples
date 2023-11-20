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

class MainWindow : public QMainWindow, ic4::QueueSinkListener
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);

	~MainWindow();

private:
	void onSelectDevice();
	void onDeviceProperties();
	void onToggleTriggerMode();
	void startstopstream();
	void onShootPhoto();
	void onStartCaptureVideo();
	void onPauseCaptureVideo();
	void onStopCaptureVideo();
	void onCodecProperties();
	void onDeviceLost();

	void updateControls();
	void updateCameraLabel();

	void createUI();

private:
	void customEvent(QEvent* event);
	void savePhoto(const ic4::ImageBuffer& imagebuffer);

	std::string _devicefile;    // File name of device state xml
	std::string _codecconfigfile;    // File name of device state xml
	std::mutex _snapphotomutex;
	bool _shootPhoto = false;
	std::atomic<bool> _capturetovideo = false;
	std::atomic<bool> _videocapturepause = false;
	QWidget* _window = nullptr;
	QGridLayout* mainLayout = nullptr;
	QWidget* _VideoWidget = nullptr;
	QAction* _DeviceSelectAct = nullptr;
	QAction* _DevicePropertiesAct = nullptr;
	QAction* _TriggerModeAct = nullptr;
	QAction* _StartLiveAct = nullptr;
	QAction* _ShootPhotoAct = nullptr;
	QAction* _recordstartact = nullptr;
	QAction* _recordpauseact = nullptr;
	QAction* _recordstopact = nullptr;
	QAction* _codecpropertyact = nullptr;
	QLabel* _sbCameralabel = nullptr;

	ic4::PropertyMap _devicePropertyMap;
	ic4::Grabber _grabber;
	std::shared_ptr<ic4::Display> _display;
	std::shared_ptr<ic4::QueueSink> _queuesink;
	ic4::VideoWriter _videowriter;

    bool sinkConnected( ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required ) final;
    void framesQueued( ic4::QueueSink& sink ) final;
};

#endif // MAINWINDOW_H
