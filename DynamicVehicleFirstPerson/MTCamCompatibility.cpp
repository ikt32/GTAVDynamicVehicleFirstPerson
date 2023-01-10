#include "MTCamCompatibility.hpp"

#include "Util/Logger.hpp"
#include "Util/Paths.hpp"

#include <simpleini/SimpleIni.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
    void MTDisableFPVCam(const fs::path& iniPath, const char* iniKey) {
        const char* iniSection = "DEBUG";

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.SetMultiLine(true);

        SI_Error result = ini.LoadFile(iniPath.string().c_str());
        if (result < 0) {
            // It doesn't exist, apparently?
            LOG(DEBUG, "[Compat] Failed to load {}", iniPath.string());
            return;
        }

        bool disabled = ini.GetBoolValue(iniSection, iniKey, false);
        if (!disabled) {
            LOG(INFO, "[Compat] Setting [{}]{} to true in {}", iniSection, iniKey, iniPath.string());
            ini.SetBoolValue(iniSection, iniKey, true);

            result = ini.SaveFile(iniPath.string().c_str());
            if (result < 0) {
                // Read-only maybe?
                LOG(WARN, "[Compat] Failed to save modified {}", iniPath.string());
                return;
            }
        }
    }
}

void Compatibility::DisableMTCam() {
    fs::path userPathMTSettings = fs::path(Paths::GetLocalAppDataPath()) /
        "ikt" / "ManualTransmission" / "settings_general.ini";

    fs::path gamePathMTSettings = fs::path(Paths::GetModuleFolder(Paths::GetOurModuleHandle())) /
        "ManualTransmission" / "settings_general.ini";

    if (fs::exists(userPathMTSettings)) {
        MTDisableFPVCam(userPathMTSettings, "FPVCamDisable");
        MTDisableFPVCam(userPathMTSettings, "DisableFPVCam");
    }

    if (fs::exists(gamePathMTSettings)) {
        MTDisableFPVCam(gamePathMTSettings, "FPVCamDisable");
        MTDisableFPVCam(gamePathMTSettings, "DisableFPVCam");
    }
}
