
#pragma once

#include <ic4/ic4.h>

#include <QString>
#include <QHBoxLayout>
#include <QApplication>
#include <QEvent>
#include <QDebug>

#include <utility>

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
		ic4::Grabber* grabber_;
		ic4::Property::NotificationToken notify_;

		QHBoxLayout* layout_;

		virtual void update_all() = 0;

	public:

		PropControlBase(TProperty prop, QWidget* parent, ic4::Grabber* grabber)
			: QWidget(parent)
			, prop_(prop)
			, grabber_(grabber)
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
		bool shoudDisplayAsLocked() const
		{
			bool prop_is_locked = prop_.isLocked(ic4::Error::Ignore());

			if (grabber_ && prop_is_locked)
			{
				if (grabber_->isStreaming() && prop_.isLikelyLockedByStream(ic4::Error::Ignore()))
				{
					return false;
				}
			}

			return prop_is_locked;
		}

		template<typename T, typename TSetFunc>
		bool propSetValue(T&& val, ic4::Error& err, TSetFunc set_func)
		{
			auto restart_info = stopStreamIfRequired(err);
			if (err.isError())
				return false;

			if (!(prop_.*set_func)(std::forward<T>(val), err))
			{
				restartStream(restart_info, ic4::Error::Ignore());
				return false;
			}

			return restartStream(restart_info, err);
		}

		template<typename TExecuteFunc>
		bool propExecute(TExecuteFunc execute_func, ic4::Error& err)
		{
			auto restart_info = stopStreamIfRequired(err);
			if (err.isError())
				return false;

			if (!(prop_.*execute_func)(err))
			{
				restartStream(restart_info, ic4::Error::Ignore());
				return false;
			}

			return restartStream(restart_info, err);
		}

	private:
		struct StreamRestartInfo
		{
			bool do_restart = false;
			ic4::StreamSetupOption stream_start_option;
			std::shared_ptr<ic4::Sink> sink;
			std::shared_ptr<ic4::Display> display;
		};

		StreamRestartInfo stopStreamIfRequired(ic4::Error& err)
		{
			if (!grabber_)
				return {};

			if (!prop_.isLikelyLockedByStream(ic4::Error::Ignore()))
				return {};

			if (!grabber_->isStreaming())
				return {};

			ic4::StreamSetupOption start_option = grabber_->isAcquisitionActive()
				? ic4::StreamSetupOption::AcquisitionStart : ic4::StreamSetupOption::DeferAcquisitionStart;
			
			auto display = grabber_->display(ic4::Error::Ignore());
			auto sink = grabber_->sink(ic4::Error::Ignore());

			if (!grabber_->streamStop(err))
				return {};

			return { true, start_option, sink, display };
		}

		bool restartStream(const StreamRestartInfo& restart_info, ic4::Error& err)
		{
			if (!grabber_)
				return true;

			if (!restart_info.do_restart)
				return true;

			return grabber_->streamSetup(restart_info.sink, restart_info.display, restart_info.stream_start_option, err);
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