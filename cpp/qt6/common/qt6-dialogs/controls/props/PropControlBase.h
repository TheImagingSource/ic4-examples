
#pragma once

#include <ic4/ic4.h>

#include <QString>
#include <QHBoxLayout>
#include <QApplication>
#include <QEvent>
#include <QDebug>
namespace ic4::ui
{
	struct IPropControl
	{
		virtual bool should_show(const QString& filter, ic4::PropVisibility visibility) = 0;
	};

	template<typename TProperty>
	class PropControlBase : public QWidget, public IPropControl
	{
	protected:
		const QEvent::Type UPDATE_ALL = static_cast<QEvent::Type>(QEvent::User + 1);
		TProperty prop_;
		ic4::Property::NotificationToken notify_;

		QHBoxLayout* layout_;

		virtual void update_all() = 0;

	public:

		PropControlBase(TProperty prop, QWidget* parent)
			: QWidget(parent)
			, prop_(prop)
		{

			layout_ = new QHBoxLayout(this);
			layout_->setSpacing(4);
			layout_->setContentsMargins(8, 7, 0, 7);
			setLayout(layout_);

			notify_ = prop_.eventAddNotification([this](ic4::Property&)
				{
					QApplication::postEvent(this, new QEvent(UPDATE_ALL));
				});

			//notify_ = prop_.eventAddNotification([this](ic4::Property&) { update_all(); });
		}
		~PropControlBase()
		{
			prop_.eventRemoveNotification(notify_);
		}

	public:
		bool should_show(const QString& filter, ic4::PropVisibility visibility) override
		{
			auto filters = filter.toLower().split(QRegularExpression(R"([(,|\|)])"));

			auto prop_display_name = QString::fromStdString(prop_.getDisplayName()).toLower();
			auto prop_name = QString::fromStdString(prop_.getName()).toLower();
			auto prop_vis = prop_.getVisibility();

			if (prop_vis > visibility)
				return false;

			if (filters.isEmpty())
				return true;

			for (auto&& f : filters)
			{
				if (prop_display_name.contains(f))
					return true;
				if (prop_name.contains(f))
					return true;
			}

			return false;
		}

	protected:
		void customEvent(QEvent* event)
		{
			if (event->type() == UPDATE_ALL)
			{
				update_all();
			}
		}
	};
}