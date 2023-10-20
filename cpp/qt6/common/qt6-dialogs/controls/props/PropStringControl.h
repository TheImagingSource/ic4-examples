
#include "PropControlBase.h"
#include "../Event.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>

namespace ic4::ui
{
	class PropStringControl : public PropControlBase<ic4::PropString>
	{
		class StringLineEdit : public QLineEdit
		{
		public:
			StringLineEdit(QWidget* parent)
				: QLineEdit(parent)
			{
			}

			app::Event<> escapePressed;
		protected:
			void keyPressEvent(QKeyEvent* e) override
			{
				if (e->key() == Qt::Key_Enter)
				{
					editingFinished();
					return;
				}
				if (e->key() == Qt::Key_Escape)
				{
					escapePressed(nullptr);
					return;
				}

				QLineEdit::keyPressEvent(e);
			}
		};

	private:
		StringLineEdit* edit_;

	public:
		PropStringControl(ic4::PropString prop, QWidget* parent, ic4::Grabber* grabber)
			: PropControlBase(prop, parent, grabber)
		{
			uint64_t max_length = (uint64_t)-1;
			try
			{
				max_length = prop.getMaxLength();
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop.getName().c_str() << " in " << iex.what();
			}


			edit_ = new StringLineEdit(this);
			edit_->setReadOnly(prop.isReadOnly());
			connect(edit_, &QLineEdit::editingFinished, this, &PropStringControl::set_value);			
			edit_->escapePressed += [this](auto) { update_value(); };
			edit_->setMaxLength(max_length);

			update_all();

			layout_->addWidget(edit_);
		}

	private:
		void set_value()
		{
			auto new_val = edit_->text().toStdString();

			ic4::Error err;
			if (!propSetValue(new_val, err, &PropString::setValue))
			{
				QMessageBox::critical(this, {}, err.message().c_str());
			}

			update_value();
		}

		void update_value()
		{
			edit_->blockSignals(true);
			try
			{
				auto val = prop_.getValue();
				edit_->setText(QString::fromStdString(val));
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop_.getName().c_str() << " in update_value() " << iex.what();
				edit_->setText("<Error>");
			}

			edit_->blockSignals(false);
		}

	protected:
		void update_all() override
		{
			update_value();

			edit_->blockSignals(true);

			bool is_readonly = prop_.isReadOnly();
			bool is_locked = prop_.isLocked();

			edit_->setSelection(0, 0);
			edit_->setReadOnly(is_readonly || is_locked);
			edit_->setEnabled(!(is_readonly || is_locked));

			// use StyleSheet in qss!
			//if (is_readonly || is_locked)
			//{
			//	edit_->setStyleSheet(R"(background-color: palette(window);)");
			//}
			//else
			//{
			//	edit_->setStyleSheet(R"(background-color: palette(base);)");
			//}
			edit_->blockSignals(false);
			edit_->update();
		}
	};
}