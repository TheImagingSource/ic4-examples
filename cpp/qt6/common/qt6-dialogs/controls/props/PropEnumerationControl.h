
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
	using EnumComboBox = app::CaptureFocus<QComboBox>;
	using EnumLineEdit = app::CaptureFocus<QLineEdit>;

	class PropEnumerationControl : public PropControlBase<ic4::PropEnumeration>
	{
	private:
		EnumComboBox* combo_ = nullptr;
		EnumLineEdit* edit_ = nullptr;

	private:
		void update_all() override
		{
			if (combo_)
			{
				combo_->blockSignals(true);
				combo_->setEnabled(!(prop_.isReadOnly(ic4::Error::Ignore()) || shoudDisplayAsLocked()));
				combo_->clear();

				bool selected_found = false;

				ic4::Error err;
				auto selected_entry = prop_.selectedEntry( err );
				if (err)
				{
					qDebug() << "Failed to query selected entry for " << prop_.name(ic4::Error::Ignore()).c_str() << ": " << err.message().c_str();
				}

				for (auto&& entry : prop_.entries(ic4::Error::Ignore()))
				{
					try
					{
						auto val = entry.intValue();

						if (!entry.isAvailable())
							continue;
						if (entry.visibility() == ic4::PropVisibility::Invisible)
							continue;

						QString name = QString::fromStdString(entry.displayName());

						combo_->addItem(name, QVariant::fromValue(val));

						if (entry == selected_entry)
						{
							combo_->setCurrentIndex(combo_->count() - 1);
							selected_found = true;
						}
					}
					catch (const ic4::IC4Exception& iex)
					{
						qDebug() << "Error " << prop_.name(ic4::Error::Ignore()).c_str() << " in update_all " << iex.what();
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
				auto selected_entry = prop_.selectedEntry(err);
				if (err.isSuccess())
				{
					edit_->setText(QString::fromStdString(selected_entry.displayName()));
				}
				else
				{
					edit_->setText("<Error>");
				}

				edit_->blockSignals(false);
			}
		}

		void comboIndexChanged(int /*index*/)
		{
			auto value = combo_->currentData().toLongLong();

			ic4::Error err;
			if (!propSetValue(value, err, &PropEnumeration::setIntValue))
			{
				QMessageBox::warning(NULL, "Set property", err.message().c_str());
				update_all();
			}
		}

	public:
		PropEnumerationControl(ic4::PropEnumeration prop, QWidget* parent, ic4::Grabber* grabber)
			: PropControlBase(prop, parent, grabber)
		{
			bool is_readonly = prop.isReadOnly();

			if (is_readonly)
			{
				edit_ = new EnumLineEdit(this);
				edit_->setReadOnly(true);

				// use StyleSheet in qss!
				//edit_->setStyleSheet(R"(background-color: palette(window);)");
			}
			else
			{
				combo_ = new EnumComboBox(this);
			}

			update_all();

			if (combo_)
			{
				connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index)
					{
						PropEnumerationControl::comboIndexChanged(index);
					});
				combo_->focus_in += [this](auto*) { onPropSelected(); };
			}
			if (edit_)
			{
				edit_->focus_in += [this](auto*) { onPropSelected(); };
			}

			if (combo_) layout_->addWidget(combo_);
			if (edit_) layout_->addWidget(edit_);
		}
	};
}
