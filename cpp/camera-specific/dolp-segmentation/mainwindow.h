#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sliderctrl.h"

#include <ic4/ic4.h> 
#include <ic4-interop/interop-Qt.h>

#include <QMainWindow>
#include <QPushButton>
#include <QtGui>

class MainWindow : public QMainWindow, ic4::QueueSinkListener
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private slots:
	void onThresholdDoLPChanged(int value);
	void onThresholdIntensityChanged(int value);

private:
	bool checkForGenTLProducers();
	void onSelectDevice();
	void onDeviceProperties();
	void startstopstream();
	void closeEvent(QCloseEvent* event);
	void updateControls();
	void createUI();
	bool isPolarizationCamera();
	bool setPolarizationFormat();
	void ThresholdPolarizedADIMono8(const ic4::ImageBuffer& src, ic4::ImageBuffer& dest);
	void ThresholdPolarizedADIRGB8(const ic4::ImageBuffer& src, ic4::ImageBuffer& dest);

	int _ThresholdDoLP = 30;
	int _ThresholdIntensity = 10;

	ic4interop::Qt::DisplayWidget* _VideoWidget = nullptr;

	QPushButton* _btnDevice = new QPushButton("Device");
	QPushButton* _btnProperties = new QPushButton("Properties");
	QPushButton* _btnLiveVideo = new QPushButton("Start");
	SliderControl* _sldThresholdDoLP = nullptr;
	SliderControl* _sldThresholdIntensity = nullptr;

	ic4::Grabber _grabber;
	std::shared_ptr<ic4::Display> _display;
	std::shared_ptr<ic4::QueueSink> _queueSink;
	std::shared_ptr<ic4::BufferPool> _bufferPool;

	bool sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required) final;
	void framesQueued(ic4::QueueSink& sink) final;
};

#endif // MAINWINDOW_H
