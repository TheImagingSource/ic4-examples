#include <QEvent>
#include <ic4/ic4.h>

const QEvent::Type GOT_PHOTO_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
const QEvent::Type DEVICE_LOST_EVENT = static_cast<QEvent::Type>(QEvent::User + 2);


/// <summary>
/// Is fired from QueueSink::framesqueued, if a photo is to be
/// saved
/// </summary>


// Define custom event subclass for new image
class GotPhotoEvent : public QEvent
{
public:
	GotPhotoEvent(std::shared_ptr<ic4::ImageBuffer> image)
		: QEvent(GOT_PHOTO_EVENT)
		, _imagebuffer(image)
	{
	}

	const ic4::ImageBuffer& getImageBuffer() const
	{
		return *_imagebuffer;
	}

private:
	std::shared_ptr<ic4::ImageBuffer> _imagebuffer;

};
