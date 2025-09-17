
#include "stream_test_camera.h"

#include <fmt/core.h>
#include <fmt/std.h>


static auto to_name(const ic4::DeviceInfo& dev_info) -> std::string
{
    auto dev_name = fmt::format("{} ({})", dev_info.modelName(), dev_info.serial());
    if (auto uid = dev_info.userID(ic4::Error::Ignore()); !uid.empty())
    {
        dev_name += fmt::format("[{}]", uid);
    }
    return dev_name;
}

static auto read_current_videoformat(ic4::Grabber& grabber) -> std::string
{
    auto map = grabber.devicePropertyMap();

    // try to set a Bayer8 format, or if not available try to set Mono8
    auto pix_format_name = map[ic4::PropId::PixelFormat].selectedEntry().name();

    auto width_val = map[ic4::PropId::Width].getValue();
    auto height_val = map[ic4::PropId::Height].getValue();
    auto framerate_val = map[ic4::PropId::AcquisitionFrameRate].getValue();

    if(auto BslResultingAcquisitionFrameRate = map.findFloat("BslResultingAcquisitionFrameRate", ic4::Error::Ignore()); BslResultingAcquisitionFrameRate.is_valid())
    {
        ic4::Error err;
        auto val = BslResultingAcquisitionFrameRate.getValue(err);
        if (!err)
        {
            framerate_val = val;
        }
    }
    return fmt::format("{} {}x{}@{:.2f}", pix_format_name, width_val, height_val, framerate_val);
}

ic4ctrl::camera_instance::camera_instance(ic4::Grabber&& dev_info)
    : grabber_(std::move(dev_info)), dev_name_(to_name(grabber_.deviceInfo()))
{
    grabber_.eventAddDeviceLost([this](ic4::Grabber&) { device_lost_ = true; });
}

auto ic4ctrl::camera_instance::create(ic4::Grabber&& grab) -> std::unique_ptr<camera_instance>
{
    return std::make_unique<camera_instance>(std::move(grab));
}

auto ic4ctrl::camera_instance::create(const std::filesystem::path& file) -> std::unique_ptr<camera_instance>
{
    ic4::Grabber grab;
    try
    {
        ic4::Error err;
        grab.deviceOpenFromState(file, err);
        if (err && err.code() != ic4::ErrorCode::Incomplete) {
            throw std::runtime_error(err.message());
        }
    }
    catch (const std::exception& ex)
    {
        fmt::println("> Failed to open/create device '{}' due to {}", file, ex.what());
        throw;
    }
    return create(std::move(grab));
}

auto ic4ctrl::camera_instance::create(const std::string& name) -> std::unique_ptr<camera_instance>
{
    ic4::Grabber grab;
    try
    {
        grab.deviceOpen(name, ic4::Error::Throw());
    }
    catch (const std::exception& ex)
    {
        fmt::println("> Failed to open/create device '{}' due to {}", name, ex.what());
        throw;
    }
    return create(std::move(grab));
}

auto ic4ctrl::camera_instance::create(const ic4::DeviceInfo& dev_info) -> std::unique_ptr<camera_instance>
{
    ic4::Grabber grab;
    try
    {
        grab.deviceOpen(dev_info, ic4::Error::Throw());
    }
    catch (const std::exception& ex)
    {
        fmt::println("> Failed to open/create device '{}' due to {}", dev_info.uniqueName(), ex.what());
        throw;
    }
    return create(std::move(grab));
}


auto ic4ctrl::camera_instance::adjust_videoformat() -> void
{
    auto map = grabber_.devicePropertyMap();

    // try to set a Bayer8 format, or if not available try to set Mono8
    auto pixel_format_prop = map[ic4::PropId::PixelFormat];

    auto select_entry_if_equal = [&](const ic4::PropEnumEntry& pixel_format_entry, ic4::PixelFormat fmt) -> bool
        {
            if (pixel_format_entry.intValue() == static_cast<uint32_t>(fmt))
            {
                pixel_format_prop.selectEntry(pixel_format_entry);
                return true;
            }
            return false;
        };
    for (auto pix : pixel_format_prop.entries())
    {
        if (select_entry_if_equal(pix, ic4::PixelFormat::BayerBG8))	break;
        if (select_entry_if_equal(pix, ic4::PixelFormat::BayerGB8))	break;
        if (select_entry_if_equal(pix, ic4::PixelFormat::BayerGR8)) break;
        if (select_entry_if_equal(pix, ic4::PixelFormat::BayerRG8))	break;
        if (select_entry_if_equal(pix, ic4::PixelFormat::Mono8))    break;
    }

    if(auto AcquisitionFrameRateEnable = map.findBoolean("AcquisitionFrameRateEnable"); AcquisitionFrameRateEnable.is_valid())
    {
        AcquisitionFrameRateEnable.setValue(true);
    }
}

auto ic4ctrl::camera_instance::setup_stream() -> bool
{
    try
    {
        StreamResendRequestedPackets_ = grabber_.driverPropertyMap().findInteger("StreamResendRequestedPackets", ic4::Error::Ignore());

        video_format_desc_ = read_current_videoformat(grabber_);

        sink_ = ic4::QueueSink::create(*this, ic4::Error::Throw());

        grabber_.streamSetup(sink_, ic4::StreamSetupOption::DeferAcquisitionStart, ic4::Error::Throw());

        PayloadSize_ = grabber_.devicePropertyMap()[ic4::PropId::PayloadSize].getValue(ic4::Error::Throw());

        fmt::println("= Device {:36} Stream: {}", dev_name_, video_format_desc_);
    }
    catch (const std::exception& ex)
    {
        fmt::println("Failed to open/create device '{}' due to {}", dev_name_, ex.what());
        throw;
    }
    return true;
}

auto ic4ctrl::camera_instance::fetch_run_statistics() -> stats
{
    auto get_value_opt = [](auto prop) {
        ic4::Error err;
        auto rval = prop.getValue(err);
        if (err) {
            return std::optional<decltype(rval)>();
        }
        return std::optional(rval);
    };
    const auto [fps,_] = fps_counter_.start_new_section();

    return {
        /*.ic4stats_ =*/ grabber_.streamStatistics(),
        /*.StreamResendRequestedPackets =*/ get_value_opt(StreamResendRequestedPackets_).value_or(-1),
        /*.mbits_per_second_ =*/ static_cast<uint64_t>((fps * PayloadSize_) * 8 / 1'000'000.f),
        /*.fps_ =*/ fps,
        /*.device_lost =*/ device_lost_.load(),
    };
}

bool ic4ctrl::camera_instance::sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& imageType, size_t min_buffers_required)
{
    return true;
}

void ic4ctrl::camera_instance::framesQueued(ic4::QueueSink& sink)
{
    fps_counter_.add_frame();
    do {
        auto buf = sink.popOutputBuffer(ic4::Error::Ignore());
        if (!buf) {
            return;
        }
    } while (true);
}
