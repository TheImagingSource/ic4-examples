
#include "../Event.h"
#include "PropControlBase.h"

#include <ic4/ic4.h>

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <algorithm>

namespace ic4::ui
{
	class PropEnumerationControl : public PropControlBase<ic4::PropEnumeration>
	{
	private:
		QComboBox* combo_ = nullptr;
		QLineEdit* edit_ = nullptr;

	private:
		void update_all() override
		{
			if (combo_)
			{
				combo_->blockSignals(true);
				combo_->setEnabled(!(prop_.isReadOnly() || shoudDisplayAsLocked()));
				combo_->clear();

				auto selected_entry = prop_.getSelectedEntry();
				bool selected_found = false;

				for (auto&& entry : prop_.getEntries())
				{
					try
					{
						auto val = entry.getIntValue();

						if (!entry.isAvailable())
							continue;

						QString name = QString::fromStdString(entry.getDisplayName());

						combo_->addItem(name, val);

						if (entry == selected_entry)
						{
							combo_->setCurrentIndex(combo_->count() - 1);
							selected_found = true;
						}
					}
					catch (const ic4::IC4Exception iex)
					{
						qDebug() << "Error " << prop_.getName().c_str() << " in update_all " << iex.what();
					}
				}

				if (!selected_found)
				{
					combo_->setCurrentIndex(-1);
				}

				combo_->blockSignals(false);
			}
			if (edit_)
			{
				edit_->blockSignals(true);

				ic4::Error err;
				auto selected_entry = prop_.getSelectedEntry(err);
				if (err.isSuccess())
				{
					edit_->setText(QString::fromStdString(selected_entry.getDisplayName()));
				}
				else
				{
					edit_->setText("<Error>");
				}

				edit_->blockSignals(false);
			}
		}

		void comboIndexChanged(int index)
		{
			auto value = combo_->currentData().toLongLong();

			ic4::Error err;
			if (!propSetValue(value, err, &PropEnumeration::setIntValue))
			{
				QMessageBox::warning(NULL, "Set property", err.message().c_str());
			}
		}

	public:
		PropEnumerationControl(ic4::PropEnumeration prop, QWidget* parent, ic4::Grabber* grabber)
			: PropControlBase(prop, parent, grabber)
		{
			bool is_readonly = prop.isReadOnly();

			if (is_readonly)
			{
				edit_ = new QLineEdit(this);
				edit_->setReadOnly(true);

				// use StyleSheet in qss!
				//edit_->setStyleSheet(R"(background-color: palette(window);)");
			}
			else
			{
				combo_ = new QComboBox(this);
			}

			update_all();

			if (combo_)
			{
				connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index)
					{
						PropEnumerationControl::comboIndexChanged(index);
					});

			}

			if (combo_) layout_->addWidget(combo_);
			if (edit_) layout_->addWidget(edit_);
		}
	};
}