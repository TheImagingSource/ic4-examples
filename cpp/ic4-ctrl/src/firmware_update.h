

#pragma once

#include <ic4/ic4.h>

#include <vector>
#include <string>

namespace ic4ctrl
{

void write_firmware_update(const std::string& firmware_file,
                           const std::string& model_overwrite,
                           bool assume_yes,
                           std::vector<ic4::DeviceInfo>& device_list);

}
