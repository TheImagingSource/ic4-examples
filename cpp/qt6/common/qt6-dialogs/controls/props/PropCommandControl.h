
#include "PropControlBase.h"

#include <QPushButton>
#include <QMessageBox>

namespace ic4::ui
{
	class PropCommandControl : public PropControlBase<ic4::PropCommand>
	{
		QPushButton* button_;

	public:
		PropCommandControl(ic4::PropCommand prop, QWidget* parent, ic4::Grabber* grabber)
			: PropControlBase(prop, parent, grabber)
		{
			std::string text = prop_.getDisplayName();

			button_ = new QPushButton(QString::fromStdString(text), this);

			connect(button_, &QPushButton::clicked, this, &PropCommandControl::execute);

			update_all();

			layout_->addWidget(button_);
		}
		
	private:
		void execute()
		{
			ic4::Error err;
			if (!propExecute(&PropCommand::execute, err))
			{
				QMessageBox::critical(this, {}, err.message().c_str());
			}
			else if (!prop_.isDone(err))
			{
				button_->setEnabled(false);
			}
		}

		void update_all() override
		{
			bool is_done = prop_.isDone(ic4::Error::Ignore());
			bool is_locked = prop_.isLocked(ic4::Error::Ignore());

			button_->setEnabled(!is_locked && is_done);
		}
	};
}