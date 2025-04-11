
#pragma once

#include <vector>
#include <functional>

#ifndef IC4_UI_BASE
#define IC4_UI_BASE 1

namespace app
{
	class IViewBase;

	template<class... Args>
	class Event
	{
	private:
		using event_delegate = std::function<void(IViewBase* sender, Args ...)>;
		std::vector<event_delegate> events_;
	public:
		Event& operator+=(event_delegate func)
		{
			events_.push_back(func);
			return *this;
		}
		void operator()(IViewBase* sender, Args ... args)
		{
			for (auto& e : events_)
			{
				e(sender, args ...);
			}
		}
		void clear()
		{
			events_.clear();
		}
	};

	template<>
	class Event<void> {};

	class IViewBase
	{
	private:
		void* tag_ = 0;
	public:
		virtual ~IViewBase() = default;

		app::Event<> onClose;

		// virtual void beginInvoke(std::function<void()> func) = 0;

		void setTag(void* val) {
			tag_ = val;
		}

		void* tag() {
			return tag_;
		}
	};

	template<typename TBase>
	class CaptureFocus : public TBase, public app::IViewBase
	{
	public:
		using TBase::TBase;
	public:
		mutable app::Event<> focus_in;
	public:
		void focusInEvent(QFocusEvent* e) override
		{
			focus_in(this);

			TBase::focusInEvent(e);
		}
	};
}
#endif 