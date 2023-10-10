
#include "PropControlBase.h"

#include <QLabel>

namespace ic4::ui
{
	class PropCategoryControl : public PropControlBase<ic4::PropCategory>
	{
	public:
		PropCategoryControl(ic4::PropCategory cat, QWidget* parent)
			: PropControlBase(cat, parent)
		{
			this->setStyleSheet("QWidget { "
				"background-color: palette(mid);"
				"}");

			auto label = new QLabel(this);
			label->setStyleSheet("QLabel { "
				"background-color: palette(mid);"
				"}");
			label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

			layout_->addWidget(label);
			layout_->setContentsMargins(0, 4, 0, 4);
		}

	protected:
		void update_all() override
		{
		}
	};
}