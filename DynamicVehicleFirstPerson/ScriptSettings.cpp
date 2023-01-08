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

    LOAD_VAL("Debug", "Enable", Debug.Enable);
    LOAD_VAL("Debug", "DisableRemoveHead", Debug.DisableRemoveHead);
    LOAD_VAL("Debug", "DisableRemoveProps", Debug.DisableRemoveProps);

    LOAD_VAL("Debug.NearClip", "Override", Debug.NearClip.Override);
    LOAD_VAL("Debug.NearClip", "Distance", Debug.NearClip.Distance);

    LOAD_VAL("Debug.DoF", "Override", Debug.DoF.Override);
    LOAD_VAL("Debug.DoF", "NearOutFocus", Debug.DoF.NearOutFocus);
    LOAD_VAL("Debug.DoF", "NearInFocus", Debug.DoF.NearInFocus);
    LOAD_VAL("Debug.DoF", "FarInFocus", Debug.DoF.FarInFocus);
    LOAD_VAL("Debug.DoF", "FarOutFocus", Debug.DoF.FarOutFocus);
}

void CScriptSettings::Save() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    SAVE_VAL("Main", "Enable", Main.Enable);

    // No save debug enable, read-only from ini
    SAVE_VAL("Debug", "DisableRemoveHead", Debug.DisableRemoveHead);
    SAVE_VAL("Debug", "DisableRemoveProps", Debug.DisableRemoveProps);

    SAVE_VAL("Debug.NearClip", "Override", Debug.NearClip.Override);
    SAVE_VAL("Debug.NearClip", "Distance", Debug.NearClip.Distance);

    SAVE_VAL("Debug.DoF", "Override", Debug.DoF.Override);
    SAVE_VAL("Debug.DoF", "NearOutFocus", Debug.DoF.NearOutFocus);
    SAVE_VAL("Debug.DoF", "NearInFocus", Debug.DoF.NearInFocus);
    SAVE_VAL("Debug.DoF", "FarInFocus", Debug.DoF.FarInFocus);
    SAVE_VAL("Debug.DoF", "FarOutFocus", Debug.DoF.FarOutFocus);

    result = ini.SaveFile(mSettingsFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save");
}
