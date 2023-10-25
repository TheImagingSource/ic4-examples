
#pragma once

#include "../Event.h"

#include "PropControlBase.h"

#include <ic4/ic4.h>

#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSignalBlocker>
#include <QLineEdit>

#include <algorithm>

namespace ic4::ui
{
	class FormattingDoubleSpinBox : public QDoubleSpinBox
	{
		ic4::PropDisplayNotation notation_;
		int precision_;

	public:
		FormattingDoubleSpinBox(QWidget* parent, ic4::PropDisplayNotation notation, int precision)
			: QDoubleSpinBox(parent)
			, notation_(notation)
			, precision_(precision)
		{
			connect(this, &QDoubleSpinBox::editingFinished, this, &FormattingDoubleSpinBox::onEditingFinished);
		}
	protected:
		QString textFromValue(double value) const override;

		void onEditingFinished()
		{
			auto text = lineEdit()->text();

			int pos = 0;
			if (validate(text, pos) != QValidator::State::Acceptable)
			{
				fixup(text);
			}

			auto val = valueFromText(text);
			setValue(val);
		}

		void keyPressEvent(QKeyEvent* e) override
		{
			if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
			{
				editingFinished();
				e->setAccepted(true);
				selectAll();
				return;
			}
			if (e->key() == Qt::Key_Escape)
			{
				QSignalBlocker blk(this);
				setValue(value());
				e->setAccepted(true);
				return;
			}

			QDoubleSpinBox::keyPressEvent(e);
		}
	};

	class PropFloatControl : public PropControlBase<ic4::PropFloat>
	{
	private:
		QSlider* slider_ = nullptr;
		FormattingDoubleSpinBox* spin_ = nullptr;

		double min_;
		double max_;
		ic4::PropFloatRepresentation representation_;

		static const int SLIDER_MIN = 0;
		static const int SLIDER_MAX = 200;
		static const int SLIDER_TICKS = SLIDER_MAX - SLIDER_MIN;

	private:
		void set_value_unchecked(double new_val)
		{
			ic4::Error err;
			if (!propSetValue(new_val, err, &PropFloat::setValue))
			{
				QMessageBox::critical(this, {}, err.message().c_str());
			}
		}

		void slider_moved(int new_pos)
		{
			std::function<double(double)> f, g;

			if (representation_ == ic4::PropFloatRepresentation::Logarithmic)
			{
				f = [](double x) { return std::log(x); };
				g = [](double x) { return std::exp(x); };
			}
			else
			{
				f = [](double x) { return x; };
				g = f;
			}

			double range_len = f(max_) - f(min_);
			double val = g(f(min_) + range_len / SLIDER_TICKS * new_pos);

			double clamped_val = std::clamp(val, min_, max_);

			set_value_unchecked(clamped_val);
		}

		int slider_position(double val)
		{
			std::function<double(double)> f;

			if (representation_ == ic4::PropFloatRepresentation::Logarithmic)
			{
				f = [](double x) { return std::log(x); };
			}
			else
			{
				f = [](double x) { return x; };
			}

			double range_len = f(max_) - f(min_);
			double p = SLIDER_TICKS / range_len * (f(val) - f(min_));

			return (int)(p + 0.5);
		}

		void update_all() override
		{
			double inc, val;
			bool has_increment = false;
			try
			{
				min_ = prop_.getMinimum();
				max_ = prop_.getMaximum();
				has_increment = prop_.getIncrementMode() == ic4::PropIncrementMode::Increment;
				if (has_increment)
				{
					inc =  prop_.getIncrement();
				}
				val = prop_.getValue();
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop_.getName().c_str() << " in update_all() " << iex.what();
				if (slider_)
					slider_->setEnabled(false);
				if (spin_)
				{
					QSignalBlocker blk(spin_);
					spin_->setEnabled(false);
					spin_->setSpecialValueText("<Error>");
					spin_->setValue(min_);
				}
				return;
			}


			bool is_locked = shoudDisplayAsLocked();
			bool is_readonly = prop_.isReadOnly();

			if (slider_)
			{
				QSignalBlocker blk(slider_);
				slider_->setMinimum(SLIDER_MIN);
				slider_->setMaximum(SLIDER_MAX);
				slider_->setValue(slider_position(val));
				slider_->setEnabled(!is_locked);
			}
			if (spin_)
			{
				QSignalBlocker blk(spin_);
				spin_->setKeyboardTracking(false);
				spin_->setSpecialValueText({});
				spin_->setMinimum(min_);
				spin_->setMaximum(max_);
				if (has_increment)
				{
					spin_->setSingleStep(inc);
				}
				spin_->setValue(val);
				spin_->setEnabled(true);
				spin_->setReadOnly(is_locked || is_readonly);
				spin_->setButtonSymbols(is_readonly ? QAbstractSpinBox::ButtonSymbols::NoButtons : QAbstractSpinBox::ButtonSymbols::UpDownArrows);
			}
		}

		void update_value(double new_value)
		{
			if (slider_)
			{
				QSignalBlocker blk(slider_);
				slider_->setValue(slider_position(new_value));
			}
			if (spin_)
			{
				QSignalBlocker blk(spin_);
				spin_->setValue(new_value);
			}
		}

	public:
		PropFloatControl(ic4::PropFloat prop, QWidget* parent, ic4::Grabber* grabber)
			: PropControlBase(prop, parent, grabber)
		{
			bool is_readonly = prop.isReadOnly();

			auto notation = prop.getDisplayNotation();
			auto precision = prop.getDisplayPrecision();
			representation_ = prop.getRepresentation();

			switch (representation_)
			{
			case ic4::PropFloatRepresentation::PureNumber:
				spin_ = new FormattingDoubleSpinBox(this, notation, precision);
				break;
			case ic4::PropFloatRepresentation::Linear:
				slider_ = is_readonly ? nullptr : new QSlider(Qt::Orientation::Horizontal, this);
				spin_ = new FormattingDoubleSpinBox(this, notation, precision);
				break;
			case ic4::PropFloatRepresentation::Logarithmic:
				slider_ = is_readonly ? nullptr : new QSlider(Qt::Orientation::Horizontal, this);
				spin_ = new FormattingDoubleSpinBox(this, notation, precision);
				spin_->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
				break;
			}

			if (slider_)
			{
				connect(slider_, &QSlider::valueChanged, this, &PropFloatControl::slider_moved);
			}
			if (spin_)
			{
				spin_->setKeyboardTracking(false);

				// TODO
				connect( spin_, QOverload<double>::of(&FormattingDoubleSpinBox::valueChanged), [this](double val) {
					set_value_unchecked(val);
				});

				spin_->setMinimumWidth(120);
				spin_->setSuffix(QString(" %1").arg(prop_.getUnit().c_str()));
			}

			update_all();

			if (slider_) layout_->addWidget(slider_);
			if (spin_) layout_->addWidget(spin_);
		}

		static QString textFromValue(double value, ic4::PropDisplayNotation notation, int precision, const QLocale& locale)
		{
			if (notation == ic4::PropDisplayNotation::Scientific)
			{
				return locale.toString(value, 'E', precision);
			}

			if (value >= std::pow(10, precision))
			{
				return locale.toString(value, 'F', 0);
			}
			else
			{
				return locale.toString(value, 'G', precision);
			}
		}
	};

	inline QString FormattingDoubleSpinBox::textFromValue(double value) const
	{
		return PropFloatControl::textFromValue(value, notation_, precision_, locale());
	}
}