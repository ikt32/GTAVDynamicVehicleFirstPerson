#include "Script.hpp"

#include "ScriptMenu.hpp"
#include "Memory/MemoryAccess.hpp"
#include "Memory/VehicleExtensions.hpp"

#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/UI.hpp"
#include "Util/Strings.hpp"

#include <inc/main.h>

namespace {
    std::shared_ptr<CFPVScript> coreScript;
    std::unique_ptr<CScriptMenu<CFPVScript>> scriptMenu;
    std::shared_ptr<CScriptSettings> settings;

    std::vector<CConfig> configs;

    bool initialized = false;
}

namespace FPV {
    void scriptInit();
    void scriptTick();

    void updateActiveConfigs();
}

void FPV::ScriptMain() {
    if (!initialized) {
        LOG(INFO, "Script started");
        scriptInit();
        initialized = true;
    }
    else {
        LOG(INFO, "Script restarted");
    }

    scriptTick();
}

void FPV::scriptInit() {
    const auto settingsGeneralPath = Paths::GetModPath() / "settings_general.ini";
    const auto settingsMenuPath = Paths::GetModPath() / "settings_menu.ini";

    settings = std::make_shared<CScriptSettings>(settingsGeneralPath.string());
    settings->Load();
    LOG(INFO, "Settings loaded");

    Memory::Init();
    VehicleExtensions::Init();
    Compatibility::Setup();

    LoadConfigs();

    coreScript = std::make_shared<CFPVScript>(settings, configs);
    coreScript->UpdateActiveConfig();

    // The menu being initialized. Note the passed settings,
    // the onInit and onExit lambdas and finally BuildMenu being called.
    scriptMenu = std::make_unique<CScriptMenu<CFPVScript>>(settingsMenuPath.string(),
        []() {
            // onInit
            settings->Load();
            LoadConfigs();
        },
        []() {
            settings->Save();
            SaveConfigs();
            // onExit
        },
        BuildMenu()
    );
}

void FPV::scriptTick() {
    while (true) {
        coreScript->Tick();
        scriptMenu->Tick(*coreScript);
        WAIT(0);
    }
}

void FPV::updateActiveConfigs() {
    if (coreScript) {
        coreScript->UpdateActiveConfig();
    }
}

CScriptSettings& FPV::GetSettings() {
    return *settings;
}

CFPVScript& FPV::GetScript() {
    return *coreScript;
}

const std::vector<CConfig>& FPV::GetConfigs() {
    return configs;
}

uint32_t FPV::LoadConfigs() {
    namespace fs = std::filesystem;

    const auto configsPath = Paths::GetModPath() / "Configs";

    LOG(DEBUG, "Reloading configs");

    configs.clear();

    if (!(fs::exists(configsPath) && fs::is_directory(configsPath))) {
        LOG(WARN, "Directory [{}] not found!", configsPath.string());
        fs::create_directories(configsPath);
    }

    for (const auto& file : fs::directory_iterator(configsPath)) {
        if (StrUtil::ToLower(fs::path(file).extension().string()) != ".ini") {
            LOG(DEBUG, "Skipping [{}] - not .ini", file.path().stem().string());
            continue;
        }

        CConfig config = CConfig::Read(fs::path(file).string());
        if (config.Name.empty()) {
            continue;
        }
        if (StrUtil::Strcmpwi(config.Name, "Default")) {
            configs.insert(configs.begin(), config);
            continue;
        }

        configs.push_back(config);
        LOG(DEBUG, "Loaded vehicle config [{}]", config.Name);
    }

    if (configs.empty() ||
        !configs.empty() && !StrUtil::Strcmpwi(configs[0].Name, "Default")) {
        LOG(WARN, "No default config found, generating a default one and saving it...");
        CConfig defaultConfig{};
        defaultConfig.Name = "Default";

        defaultConfig.Mount.push_back(CConfig::SCameraSettings{
                .Name = "Default",
                .Order = 0
            }
        );
        configs.insert(configs.begin(), defaultConfig);
        defaultConfig.Write(CConfig::ESaveType::GenericNone);
    }

    LOG(INFO, "Configs loaded: {}", configs.size());

    FPV::updateActiveConfigs();
    return static_cast<unsigned>(configs.size());
}

void FPV::SaveConfigs() {
    namespace fs = std::filesystem;

    const auto configsPath = Paths::GetModPath() / "Configs";

    LOG(DEBUG, "Saving all configs");

    if (!(fs::exists(configsPath) && fs::is_directory(configsPath))) {
        LOG(ERROR, "Directory [{}] not found!", configsPath.string());
        return;
    }

    for (auto& config : configs) {
        CConfig::ESaveType saveType;
        if (config.Name == "Default") {
            saveType = CConfig::ESaveType::GenericNone;
        }
        else if (config.Plate.empty()) {
            saveType = CConfig::ESaveType::GenericModel;
        }
        else {
            saveType = CConfig::ESaveType::Specific;
        }

        config.Write(saveType);
    }
}
