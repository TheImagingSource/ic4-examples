#pragma once

#include <ic4/ic4.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "fps_counter.h"

namespace ic4ctrl
{
    struct camera_instance : ic4::QueueSinkListener
    {
        static auto create(ic4::Grabber&& grab) -> std::unique_ptr<camera_instance>;
        static auto create(const ic4::DeviceInfo& dev_info) -> std::unique_ptr<camera_instance>;
        static auto create(const std::string& name) -> std::unique_ptr<camera_instance>;
        static auto create(const std::filesystem::path& file) -> std::unique_ptr<camera_instance>;

        camera_instance(ic4::Grabber&& dev_info);

        auto adjust_videoformat() -> void;

        auto setup_stream() -> bool;

        void start() { grabber_.acquisitionStart(); }
        void stop() { grabber_.acquisitionStop(); }

        struct stats
        {
            ic4::Grabber::StreamStatistics ic4stats_{};
            int64_t StreamResendRequestedPackets = -1;
            uint64_t mbits_per_second_ = 0;

            float fps_ = 0.f;
            bool device_lost = false;

            auto fps() const noexcept -> float { return fps_; }
            auto calc_MBitsPerSeconds() const noexcept -> uint64_t { return mbits_per_second_; }
        };

        auto fetch_run_statistics() -> stats;

        auto name() { return dev_name_; }
        auto vid_info() { return video_format_desc_; }
    private:
        ic4::Grabber grabber_;

        std::string	dev_name_;
        std::string video_format_desc_;
        std::shared_ptr<ic4::Sink>	sink_;

        std::atomic<bool>   device_lost_ = false;

        int64_t PayloadSize_ = 0;

        img_lib::fps_counter_section   fps_counter_;

        ic4::PropInteger StreamResendRequestedPackets_;
    public:
        bool sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required) final;
        void framesQueued(ic4::QueueSink& sink) final;
    };
}

