
#pragma once

#include <atomic>
#include <chrono>

namespace img_lib
{
	struct fps_counter_section
    {
        /* Adds a frame for the frame counter and returns true when the measure interval was reached. Note: this does not update the average frame time */
        auto    add_frame() noexcept -> void {
            if (frame_count_ == 0) {
                start_time_ = std::chrono::steady_clock::now();
            }
            ++frame_count_;
        }

        struct result 
        {
            float fps = 0.;
            uint64_t count = 0;
        };

        /* end the fps section */
        auto    calc_section_fps(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const noexcept -> result
        {
            using namespace std::chrono;

            const auto diff_in_us = duration_cast<microseconds>(now - start_time_).count();

            const auto cnt = frame_count_.load();

            float fps = 0.;
            if (diff_in_us > 0) {
                fps = (static_cast<float>(cnt) * 1'000'000.0f) / static_cast<float>(diff_in_us);
            }
            return { fps, cnt };
        }

        auto start_new_section(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) noexcept -> result
        {
            auto res = calc_section_fps(now);
            frame_count_ = 0;
            return res;
        }
    private:
        std::chrono::steady_clock::time_point   start_time_;
        std::atomic<uint64_t>	                frame_count_ = 0;
    };
}
