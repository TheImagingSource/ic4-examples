
#include "PropControlBase.h"

#include <QPushButton>

namespace ic4::ui
{
	class PropCommandControl : public PropControlBase<ic4::PropCommand>
	{
		QPushButton* button_;

	public:
		PropCommandControl(ic4::PropCommand prop, QWidget* parent)
			: PropControlBase(prop, parent)
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
			prop_.execute();
		}

		void update_all() override
		{
			button_->setEnabled(!prop_.isLocked());
		}
	};
}