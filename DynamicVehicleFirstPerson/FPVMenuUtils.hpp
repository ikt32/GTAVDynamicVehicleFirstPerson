#pragma once

#include "Config.hpp"
#include <inc/types.h>
#include <string>
#include <vector>

namespace FPV {
    void CreateConfig(CConfig& config, Vehicle vehicle);
    std::string MountName(CConfig::EMountPoint mount);

    std::vector<std::string> FormatConfigInfo(const CConfig& cfg);
    std::vector<std::string> FormatCameraInfo(const CConfig& cfg);

    std::string GetKbEntryString(const std::string& existingString);
    bool GetKbEntryInt(int& val);
    bool GetKbEntryFloat(float& val);
}
