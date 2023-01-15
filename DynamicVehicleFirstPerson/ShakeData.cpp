#include "ShakeData.hpp"
#include "SettingsCommon.hpp"

#include "Util/Logger.hpp"

#include <simpleini/SimpleIni.h>
#include <format>

#define CHECK_LOG_SI_ERROR(result, operation) \
    if (result < 0) { \
        LOG(ERROR, "[Shake] {} Failed to {}, SI_Error [{}]", \
        __FUNCTION__, operation, result); \
    }

#define LOAD_VAL(section, key, option) \
    option = GetValue(ini, section, key, option)

CShakeData::CShakeData(const std::string & shakeFile)
    : mShakeFile(shakeFile) {
}

void CShakeData::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mShakeFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load");

    LOAD_VAL("BaseRates", "MinRateModSpd", MinRateModSpd);
    LOAD_VAL("BaseRates", "MaxRateModSpd", MaxRateModSpd);
    LOAD_VAL("BaseRates", "MinRateModTrn", MinRateModTrn);
    LOAD_VAL("BaseRates", "MaxRateModTrn", MaxRateModTrn);

    MaterialReactionMap.clear();
    for (uint32_t i = 0; i < sMaterialNames.size(); ++i) {
        if (ini.KeyExists("MaterialReaction", sMaterialNames[i])) {
            auto line = ini.GetValue("MaterialReaction", sMaterialNames[i]);
            float amplitude, frequency;
            auto scanned = sscanf_s(line, "%f|%f", &amplitude, &frequency);
            if (scanned != 2) {
                LOG(ERROR, "[Shake] Failed to process line: '{}'", line);
                continue;
            }
            MaterialReactionMap.insert({ static_cast<eMaterial>(i), { amplitude, frequency } });
        }
    }
}
