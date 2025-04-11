
#pragma once

#include <ic4/ic4.h>

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QApplication>
#include <QEvent>
#include <QDebug>
#include <QTime>
#include <QTimer>

#include <utility>

namespace ic4::ui
{
	struct StreamRestartInfo
	{
		bool do_restart = false;
		ic4::StreamSetupOption stream_start_option;
		std::shared_ptr<ic4::Sink> sink;
		std::shared_ptr<ic4::Display> display;
	};

	using StreamRestartFilterFunction = std::function<StreamRestartInfo(ic4::Grabber& grabber, const StreamRestartInfo& info)>;
	using PropSelectedFunction = std::function<void(ic4::Property& prop)>;

	struct IPropControl
	{
		virtual ~IPropControl() = default;
		virtual bool should_show(const QString& filter, ic4::PropVisibility visibility) = 0;
		virtual void registerPropSelected(PropSelectedFunction fn) = 0;
		virtual void registerStreamRestartFilter(StreamRestartFilterFunction fn) = 0;
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

		QTime prev_update_;
		QTimer final_update_;
		StreamRestartFilterFunction restartFilterFunc_ = nullptr;
		PropSelectedFunction propSelectedFunc_ = nullptr;

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

			final_update_.setSingleShot(true);
			final_update_.setInterval(100);
			final_update_.callOnTimeout(
				[this]()
				{
					QApplication::removePostedEvents(this, UPDATE_ALL);
					QApplication::postEvent(this, new QEvent(UPDATE_ALL));
				}
			);

			notify_ = prop_.eventAddNotification(
				[this](ic4::Property&)
				{
					QApplication::removePostedEvents(this, UPDATE_ALL);
					QApplication::postEvent(this, new QEvent(UPDATE_ALL));
				}
			);
		}
		~PropControlBase()
		{
			prop_.eventRemoveNotification(notify_, ic4::Error::Ignore());
		}

	public:
		bool should_show(const QString& filter, ic4::PropVisibility visibility) override
		{
			auto filters = filter.toLower().split(QRegularExpression(R"([(,|\|)])"));

			auto prop_display_name = QString::fromStdString(prop_.displayName()).toLower();
			auto prop_name = QString::fromStdString(prop_.name()).toLower();
			auto prop_vis = prop_.visibility();

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

	public:
		void registerPropSelected(PropSelectedFunction fn) final
		{
			propSelectedFunc_ = fn;
		}
		void registerStreamRestartFilter(StreamRestartFilterFunction fn) final
		{
			restartFilterFunc_ = fn;
		}

	protected:
		void onPropSelected()
		{
			if (propSelectedFunc_)
			{
				propSelectedFunc_(prop_);
			}
		}

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

			StreamRestartInfo info = restart_info;

			if (restartFilterFunc_)
			{
				info = restartFilterFunc_(*grabber_, info);
			}

			return grabber_->streamSetup(info.sink, info.display, info.stream_start_option, err);
		}

	protected:
		void customEvent(QEvent* event)
		{
			if (event->type() == UPDATE_ALL)
			{
				if (QTime::currentTime() > prev_update_.addMSecs(66))
				{
					update_all();

					prev_update_ = QTime::currentTime();

					final_update_.stop();
				}
				else
				{
					final_update_.start();
				}
			}
		}
	};
}