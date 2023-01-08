#include "ScriptSettings.hpp"
#include "SettingsCommon.hpp"

#include "Util/Logger.hpp"

#include <simpleini/SimpleIni.h>

#define CHECK_LOG_SI_ERROR(result, operation) \
    if (result < 0) { \
        LOG(ERROR, "[Settings] {} Failed to {}, SI_Error [{}]", \
        __FUNCTION__, operation, result); \
    }

#define SAVE_VAL(section, key, option) \
    SetValue(ini, section, key, option)

#define LOAD_VAL(section, key, option) \
    option = GetValue(ini, section, key, option)

CScriptSettings::CScriptSettings(std::string settingsFile)
    : mSettingsFile(std::move(settingsFile)) {

}

void CScriptSettings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    LOAD_VAL("Main", "Enable", Main.Enable);
    LOAD_VAL("Debug", "DisableRemoveHead", Debug.DisableRemoveHead);
    LOAD_VAL("Debug", "DisableRemoveProps", Debug.DisableRemoveProps);
}

void CScriptSettings::Save() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    SAVE_VAL("Main", "Enable", Main.Enable);
    SAVE_VAL("Debug", "DisableRemoveHead", Debug.DisableRemoveHead);
    SAVE_VAL("Debug", "DisableRemoveProps", Debug.DisableRemoveProps);

    result = ini.SaveFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save");
}
