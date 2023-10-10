
#pragma once

#include "../Event.h"

#include "PropIntSpinBox.h"
#include "PropControlBase.h"

#include <ic4/ic4.h>

#include <QWidget>
#include <QCheckBox>
#include <QSlider>

#include <algorithm>

namespace ic4::ui
{
	class PropIntControl : public PropControlBase<ic4::PropInteger>
	{
	private:
		QCheckBox* check_ = nullptr;
		QSlider* slider_ = nullptr;
		PropIntSpinBox* spin_ = nullptr;
		QLineEdit* edit_ = nullptr;

		ic4::PropIntRepresentation representation_;
		int64_t min_;
		int64_t max_;
		int64_t inc_;
		int64_t val_;

	private:
		static QString format_ip(uint32_t ip)
		{
			return QString("%1.%2.%3.%4")
				.arg((ip >> 24) & 0xFF)
				.arg((ip >> 16) & 0xFF)
				.arg((ip >> 8) & 0xFF)
				.arg((ip >> 0) & 0xFF);
		}
		static QString format_mac(uint64_t mac)
		{
			return QString("%1:%2:%3:%4:%5:%6")
				.arg((mac >> 40) & 0xFF, 2, 16, (QChar)'0')
				.arg((mac >> 32) & 0xFF, 2, 16, (QChar)'0')
				.arg((mac >> 24) & 0xFF, 2, 16, (QChar)'0')
				.arg((mac >> 16) & 0xFF, 2, 16, (QChar)'0')
				.arg((mac >> 8) & 0xFF, 2, 16, (QChar)'0')
				.arg((mac >> 0) & 0xFF, 2, 16, (QChar)'0');
		}

	public:
		static QString value_to_string(int64_t val, ic4::PropIntRepresentation rep)
		{
			switch (rep)
			{
			case ic4::PropIntRepresentation::Boolean:
				return val ? "True" : "False";
			case ic4::PropIntRepresentation::HexNumber:
				return QString("0x%1").arg(val, 0, 16);
			case ic4::PropIntRepresentation::PureNumber:
			case ic4::PropIntRepresentation::Linear:
			case ic4::PropIntRepresentation::Logarithmic:
			default:
				return QString::number(val);			
			case ic4::PropIntRepresentation::MACAddress:
				return format_mac(val);
			case ic4::PropIntRepresentation::IPV4Address:
				return format_ip(val);
			}
		}

	private:
		void set_value_unchecked(int64_t new_val)
		{
			try
			{
				auto testVal = prop_.getValue();

				prop_.setValue(new_val);

				val_ = prop_.getValue();

				update_value(val_);
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop_.getName().c_str() << " in set_value_unchecked() " << iex.what();
			}
		}

		void set_value(int new_pos)
		{
			int64_t new_val = std::clamp(static_cast<int64_t>(new_pos), min_, max_);

			if ((new_val - min_) % inc_)
			{
				auto fixed_val = min_ + (new_val - min_) / inc_ * inc_;

				if (fixed_val == val_)
				{
					if (new_val > val_)
						new_val = val_ + inc_;
					if (new_val < val_)
						new_val = val_ - inc_;
				}
				else
				{
					new_val = fixed_val;
				}
			}

			set_value_unchecked(new_val);
		}

		void update_all() override
		{
			try
			{
				min_ = prop_.getMinimum();
				max_ = prop_.getMaximum();
				inc_ = prop_.getIncrement();
				val_ = prop_.getValue();
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop_.getName().c_str() << " : update_all() " << iex.what();
				spin_->setEnabled(false);
				return;
			}


			bool is_locked = prop_.isLocked();
			bool is_readonly = prop_.isReadOnly();

			auto int_min_i64 = static_cast<int64_t>(std::numeric_limits<int>::min());
			auto int_max_i64 = static_cast<int64_t>(std::numeric_limits<int>::max());

			int qt_min = static_cast<int>(std::clamp(min_, int_min_i64, int_max_i64));
			int qt_max = static_cast<int>(std::clamp(max_, int_min_i64, int_max_i64));

			if (slider_)
			{
				slider_->blockSignals(true);
				slider_->setMinimum(qt_min);
				slider_->setMaximum(qt_max);
				slider_->setSingleStep(inc_);
				slider_->setValue(val_);
				slider_->setEnabled(!is_locked);
				slider_->blockSignals(false);				
			}
			if (spin_)
			{			

				spin_->blockSignals(true);
				spin_->setMinimum(min_);
				spin_->setMaximum(max_);
				spin_->setSingleStep(inc_);
				spin_->setValue(val_);																
				spin_->setReadOnly(is_locked || is_readonly);

				spin_->setEnabled( !(is_locked || is_readonly));
				// use StyleSheet in qss!
				/*
				if (is_locked || is_readonly)
				{
					spin_->setStyleSheet(R"(background-color: palette(window);)");
				}
				else
				{
					spin_->setStyleSheet(R"(background-color: palette(base);)");
				}*/

				spin_->setButtonSymbols(is_readonly ? QAbstractSpinBox::ButtonSymbols::NoButtons : QAbstractSpinBox::ButtonSymbols::UpDownArrows);
				spin_->blockSignals(false);
			}
			if (edit_)
			{
				edit_->blockSignals(true);

				edit_->setText(value_to_string(val_, representation_));

				edit_->setReadOnly(is_locked || is_readonly);

				// use StyleSheet in qss!
				/*if (is_locked || is_readonly)
				{
					edit_->setStyleSheet(R"(background-color: palette(window);)");
				}
				else
				{
					edit_->setStyleSheet(R"(background-color: palette(base);)");
				}*/
				edit_->blockSignals(false);
			}
		}

		void update_value(int64_t new_value)
		{
			if (slider_)
			{
				slider_->blockSignals(true);
				slider_->setValue(new_value);
				slider_->blockSignals(false);
			}
			if (spin_)
			{
				spin_->blockSignals(true);
				spin_->setValue(new_value);
				spin_->blockSignals(false);
			}
		}

	public:
		PropIntControl(ic4::PropInteger prop, QWidget* parent)
			: PropControlBase(prop, parent)
		{
			bool is_readonly = prop.isReadOnly();

			representation_ = prop.getRepresentation();

			switch (representation_)
			{
			case ic4::PropIntRepresentation::Boolean:
				throw "not implemented";
				check_ = new QCheckBox(this);
				break;
			case ic4::PropIntRepresentation::HexNumber:
				spin_ = new PropIntSpinBox(this, 16);
				spin_->setPrefix("0x");
				break;
			case ic4::PropIntRepresentation::PureNumber:
				spin_ = new PropIntSpinBox(this);
				break;
			case ic4::PropIntRepresentation::Linear:
				slider_ = is_readonly ? nullptr : new QSlider(Qt::Orientation::Horizontal, this);
				spin_ = new PropIntSpinBox(this);
				break;
			case ic4::PropIntRepresentation::Logarithmic:
				slider_ = is_readonly ? nullptr : new QSlider(Qt::Orientation::Horizontal, this);
				spin_ = new PropIntSpinBox(this);
				printf("not implemented: IC4_PROPINTREP_LOGARITHMIC\n");
				break;
			case ic4::PropIntRepresentation::MACAddress:
				printf("not implemented: IC4_PROPINTREP_MACADDRESS\n");
				edit_ = new QLineEdit(this);
				break;
			case ic4::PropIntRepresentation::IPV4Address:
				printf("not implemented: IC4_PROPINTREP_IPV4ADDRESS\n");
				edit_ = new QLineEdit(this);
				break;
			}

			if (slider_)
			{
				connect(slider_, &QSlider::valueChanged, this, &PropIntControl::set_value);
			}
			if (spin_)
			{
				spin_->setKeyboardTracking(false);
				//connect(spin_, &QSpinBox::valueChanged, this, &PropIntControl::set_value);

				spin_->value_changed += [this](auto* /* sender */, auto v) { set_value_unchecked(v); };
				spin_->setMinimumWidth(120);
				spin_->setSuffix(QString("%1").arg(prop_.getUnit().c_str()));
			}

			update_all();

			if (check_) layout_->addWidget(check_);
			if (slider_) layout_->addWidget(slider_);
			if (spin_) layout_->addWidget(spin_);
			if (edit_) layout_->addWidget(edit_);		
		}
	};
}