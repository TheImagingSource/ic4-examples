
#include <ic4/ic4.h>

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>

#include <cstdint>
#include <atomic>
#include <mutex>

class HighSpeedCaptureDialog : public QDialog, public ic4::QueueSinkListener
{
public:
	HighSpeedCaptureDialog();

private:
	void createUI();
	void updateUI();

	void readSettings();
	void saveSettings();

private:
	void onSelectDevice();
	void onDeviceProperties();
	void onStartStop();

private:
	void customEvent(QEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	bool sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required) final;
	void framesQueued(ic4::QueueSink& sink) final;

private:
	ic4::Grabber _grabber;
	std::shared_ptr<ic4::Display> _display;
	std::shared_ptr<ic4::QueueSink> _sink;
	int _frame_number;

	std::mutex _frames_queued_mtx;

	std::atomic<int64_t> _num_processed;
	std::atomic<int64_t> _num_total;
	std::atomic<int64_t> _num_free;
	std::atomic<int64_t> _num_filled;

	std::atomic<bool> _cancel_cleanup = false;
	std::atomic<bool> _cleanup_active = false;

	QPushButton* _selectDevice = nullptr;
	QPushButton* _deviceProperties = nullptr;
	QLineEdit* _destinationDirectory = nullptr;
	QPushButton* _destinationBrowse = nullptr;
	QLineEdit* _bufferMemory = nullptr;
	QProgressBar* _freeBuffersProgress = nullptr;
	QLabel* _freeBuffersLabel = nullptr;
	QProgressBar* _filledBuffersProgress = nullptr;
	QLabel* _filledBuffersLabel = nullptr;
	QPushButton* _startStop = nullptr;
	QLabel* _captureInfo = nullptr;
};