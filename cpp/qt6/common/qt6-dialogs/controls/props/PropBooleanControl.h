
#include "PropControlBase.h"

namespace ic4::ui
{
	class PropBooleanControl : public PropControlBase<ic4::PropBoolean>
	{
		QCheckBox* check_;

	public:
		PropBooleanControl(ic4::PropBoolean prop, QWidget* parent)
			: PropControlBase(prop, parent)
		{
			check_ = new QCheckBox(this);
			check_->setText("");

			// use stylesheet in qss - breaks checkbox images used in qss!
			//check_->setStyleSheet("QCheckBox::indicator { width: 16px; height: 16px; }");

			connect(check_, &QCheckBox::stateChanged, this, &PropBooleanControl::check);

			update_all();

			layout_->addWidget(check_);
			layout_->setContentsMargins(8, 8, 0, 8);
		}

	private:
		void check(int new_state)
		{
			prop_.setValue(new_state == Qt::Checked);
		}

		void update_all() override
		{
			check_->setEnabled(!prop_.isLocked() && !prop_.isReadOnly());
			check_->blockSignals(true);

			try
			{
				auto value = prop_.getValue();
				check_->setChecked(value);
			}
			catch (const ic4::IC4Exception iex)
			{
				qDebug() << "Error " << prop_.getName().c_str() << " in update_all " << iex.what();
				check_->setEnabled(false);
			}

			check_->blockSignals(false);
		}
	};
}