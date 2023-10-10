#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QtGui>
#include <QAction>
#include <QLabel>

#include <mutex>

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
	void createUI();

private:
	void customEvent(QEvent* event);
	void savePhoto(const ic4::ImageBuffer& imagebuffer);

	std::string _name;          // Name of the application
	std::string _devicefile;    // File name of device state xml
	std::string _codecconfigfile;    // File name of device state xml
	bool _shootPhoto;
	bool _capturetovideo;
	bool _videocapturepause;
	QWidget* _window;
	QGridLayout* mainLayout;
	QWidget* _VideoWidget;
	QAction* _DeviceSelectAct;
	QAction* _DevicePropertiesAct;
	QAction* _TriggerModeAct;
	QAction* _StartLiveAct;
	QAction* _ShootPhotoAct;
	QAction* _recordstartact;
	QAction* _recordpauseact;
	QAction* _recordstopact;
	QAction* _codecpropertyact;

	QLabel* _sbCameralabel;
	ic4::Grabber _grabber;
	std::shared_ptr<ic4::Display> _display;
	std::shared_ptr<ic4::QueueSink> _queuesink;
	ic4::VideoWriter _videowriter;
	std::mutex _snapphotomutex;

	virtual bool sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t  	min_buffers_required);

	virtual void framesQueued(ic4::QueueSink& sink);
};

#endif // MAINWINDOW_H
