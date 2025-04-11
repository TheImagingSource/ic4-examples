
#include "../Event.h"

#include <QSlider>
#include <QKeyEvent>
#include <QDebug>
#include <QGuiApplication>

#include <cstdint>
#include <optional>
#include <charconv>
#include <algorithm>

namespace ic4::ui
{
	class PropIntSlider : public app::CaptureFocus<QSlider>
	{
	private:
		const int SLIDER_MAX = 10000;
		int64_t val_ = 0;
		int64_t min_ = 0;
		int64_t max_ = 99;
	public:
		PropIntSlider(QWidget* parent)
			: app::CaptureFocus<QSlider>(Qt::Orientation::Horizontal, parent)
		{
			connect(this, &QSlider::valueChanged, this, &PropIntSlider::onValueChanged);

			QSlider::setMinimum(0);
			QSlider::setMaximum(SLIDER_MAX);
			QSlider::setSingleStep(1);
		}
	public:
		mutable app::Event<int64_t> value_changed;
		mutable app::Event<int64_t> value_step;
	private:
		using QSlider::setMinimum;
		using QSlider::setMaximum;
		using QSlider::setSingleStep;
	public:
		void setRange(int64_t min, int64_t max)
		{
			min_ = min;
			max_ = max;
			updatePosition();
		}
		void setValue(int64_t val)
		{
			val_ = val;
			updatePosition();
		}
	private:
		int64_t sliderToValue(int slider_pos)
		{
			auto rel = slider_pos / static_cast<double>(SLIDER_MAX);
			auto range_len = static_cast<uint64_t>(max_ - min_);
			auto val = min_ + range_len* rel;
			if (val >= static_cast<double>(max_))
				return max_;
			if (val <= static_cast<double>(min_))
				return min_;
			return static_cast<int64_t>(val);
		}
		int valueToSlider(int64_t value)
		{
			auto offset = static_cast<uint64_t>(value - min_);
			auto rel = offset / static_cast<double>(static_cast<uint64_t>(max_ - min_));
			auto pos = static_cast<int64_t>(SLIDER_MAX * rel);
			return pos;
		}
	private:
		void onValueChanged(int slider_pos)
		{
			val_ = sliderToValue(slider_pos);
			value_changed(this, val_);
		}
		void updatePosition()
		{
			auto sliderPos = valueToSlider(val_);

			QSignalBlocker blk(this);
			QSlider::setValue(sliderPos);
		}
	protected:
		void keyPressEvent(QKeyEvent* ev) final
		{
			int mag = 1;
			if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				mag *= 10;

			if (ev->key() == Qt::Key_Left || ev->key() == Qt::Key_Down)
			{
				value_step(this, -mag);
				ev->accept();
			}
			else if (ev->key() == Qt::Key_Right || ev->key() == Qt::Key_Up)
			{
				value_step(this, mag);
				ev->accept();
			}
			else if (ev->key() == Qt::Key_PageUp)
			{
				value_step(this, 10 * mag);
				ev->accept();
			}
			else if (ev->key() == Qt::Key_PageDown)
			{
				value_step(this, -10 * mag);
				ev->accept();
			}
			else
			{
				return QSlider::keyPressEvent(ev);
			}
		}
		void wheelEvent(QWheelEvent* e) final
		{
			int mag = 1;
			if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				mag *= 10;			

			int delta = e->angleDelta().y();
			if (delta > 0)
			{
				value_step(this, mag);
				e->accept();
			}
			else if (delta < 0)
			{
				value_step(this, -mag);
				e->accept();
			}
		}
	};
}