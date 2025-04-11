
#include "../Event.h"

#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QKeyEvent>

#include <cstdint>
#include <optional>
#include <charconv>
#include <QDebug>
namespace ic4::ui
{
	class PropIntSpinBox : public app::CaptureFocus<QAbstractSpinBox>
	{
	private:
		mutable int64_t val_ = 0;
		int64_t min_ = 0;
		int64_t max_ = 99;
		int64_t inc_ = 1;
		QString prefix_;
		QString suffix_;
		int number_base_ = 10;
	public:
		PropIntSpinBox(QWidget* parent, int number_base = 10)
			: app::CaptureFocus<QAbstractSpinBox>(parent)
			, number_base_(number_base)
		{
			connect(this, &QAbstractSpinBox::editingFinished, this, &PropIntSpinBox::parse_new_text);
		}
	public:
		mutable app::Event<int64_t> value_changed;
		mutable app::Event<int64_t> value_step;
	public:
		void setMinimum(int64_t min)
		{
			min_ = min;
		}
		void setMaximum(int64_t max)
		{
			max_ = max;
		}
		void setSingleStep(int64_t inc)
		{
			inc_ = inc;
		}
		void setValue(int64_t val)
		{
			val_ = val;
			lineEdit()->setText(build_text(val_));
		}
		void setSuffix(QString suffix)
		{
			suffix_ = suffix;
		}
		void setPrefix(QString prefix)
		{
			prefix_ = prefix;
		}
	private:
		void set_value_internal(int64_t val, bool event_if_changed = true) const
		{
			bool changed = (val != val_);
			val_ = val;

			if (changed && event_if_changed)
			{
				value_changed(const_cast<PropIntSpinBox*>(this), val_);
			}
		}

		QString remove_suffix(const QString& val) const
		{
			auto stdStr = val.toStdString();

			auto tmp = val.trimmed();
			int begin = 0;
			int end = tmp.size();
			if (tmp.startsWith(prefix_))
				begin = prefix_.size();
			if (tmp.endsWith(suffix_))
				end = tmp.size() - suffix_.size();

			return tmp.mid(begin, end).trimmed();
		}

		std::optional<int64_t> parse_number(const QString& input) const
		{
			auto s = input.toStdString();
			int64_t val;
			const auto* first = s.data();
			const auto* last = first + s.length();
			auto res = std::from_chars(first, last, val, number_base_);

			if (res.ec != std::errc() || res.ptr != last)
				return std::nullopt;

			return val;
		}

		auto parse_text(QString text) const
		{
			struct ParseResult
			{
				bool is_empty;
				bool is_valid;
				int64_t value;
			};

			ParseResult r = {};

			auto without_suffix = remove_suffix(text);
			if (without_suffix.isEmpty())
			{
				r.is_empty = true;
				return r;
			}

			auto res = parse_number(without_suffix);
			if (!res.has_value())
			{
				r.is_valid = false;
				return r;
			}

			r.is_valid = true;
			r.value = res.value();
			return r;
		}

		void parse_new_text()
		{
			if (isReadOnly())
				return;

//#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
			if (!lineEdit()->isModified())
				return; 

			lineEdit()->setModified(false);
//#endif 

			auto text = lineEdit()->text();

			int pos = 0;
			if (validate(text, pos) != QValidator::State::Acceptable)
			{
				fixup(text);
			}

			auto res = parse_text(text);
			if (res.is_valid)
			{
				value_changed(this, res.value);
			}
		}

		QString build_text(int64_t val) const
		{
			auto s = prefix_ + QString::number(val, number_base_).toUpper();

			if (!suffix_.isEmpty())
				s += " " + suffix_;

			return s;
		}
	protected:
		QAbstractSpinBox::StepEnabled stepEnabled() const override
		{
			StepEnabled result = StepUpEnabled | StepDownEnabled;

			if (val_ == min_)
				result &= ~StepDownEnabled;
			if (val_ == max_)
				result &= ~StepUpEnabled;

			return result;
		}

		QValidator::State validate(QString& input, int& /* pos */) const override
		{
			auto res = parse_text(input);

			if (res.is_empty)
				return QValidator::State::Intermediate;

			if (!res.is_valid)
				return QValidator::State::Invalid;

			if (res.value < min_)
				return QValidator::State::Intermediate;

			if (res.value > max_)
				return QValidator::State::Intermediate;

			if ((res.value - min_) % inc_)
				return QValidator::State::Intermediate;

			return QValidator::State::Acceptable;
		}

		void fixup(QString& input) const override
		{
			auto res = parse_text(input);

			if (res.is_empty)
			{
				input = build_text(min_);
				return;
			}

			if (!res.is_valid)
				return;

			int64_t val = res.value;

			val = std::clamp(val, min_, max_);

			if ((val - min_) % inc_)
			{
				val = min_ + (val - min_) / inc_ * inc_;
			}

			input = build_text(val);
		}

		void stepBy(int steps) override
		{
			if (isReadOnly())
				return;

			auto text = lineEdit()->text();
			fixup(text);

			value_step(this, steps);
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
				lineEdit()->setText(build_text(val_));
				e->setAccepted(true);
				return;
			}

			QAbstractSpinBox::keyPressEvent(e);
		}
	};
}