
#include <cstdint>
#include <chrono>

namespace ic4demoapp
{
	struct FpsCounter
	{
	public:
		FpsCounter(std::chrono::duration<uint64_t> update_interval = std::chrono::seconds(1))
			: update_interval_(update_interval)
		{
		}

	public:
		void notify_frame()
		{
			if (count_ < 0)
			{
				prev_update_ = std::chrono::high_resolution_clock::now();
				count_ = 0;
			}
			else
			{
				++count_;
				update_if_required(update_interval_);
			}
		}

		double current()
		{
			if (count_ < 0)
				return 0;

			update_if_required(update_interval_ * 3);
			return current_;
		}

	private:
		void update_if_required(std::chrono::duration<uint64_t> interval)
		{
			auto now = std::chrono::high_resolution_clock::now();
			if (now > prev_update_ + interval)
			{
				auto dt_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev_update_).count();
				current_ = 1e9 * static_cast<double>(count_) / static_cast<double>(dt_ns);

				prev_update_ = now;
				count_ = 0;
			}
		}

		std::chrono::duration<uint64_t> update_interval_;

		int64_t count_ = -1;
		std::chrono::time_point<std::chrono::high_resolution_clock> prev_update_ = {};

		double current_ = 0;
	};
}