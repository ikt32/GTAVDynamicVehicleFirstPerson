#include "SettingsCommon.hpp"

void SetValue(CSimpleIniA& ini, const std::string& section, const char* key, int val) {
    ini.SetLongValue(section.c_str(), key, val);
}

void SetValue(CSimpleIniA& ini, const std::string& section, const char* key, const std::string& val) {
    ini.SetValue(section.c_str(), key, val.c_str());
}

void SetValue(CSimpleIniA& ini, const std::string& section, const char* key, bool val) {
    ini.SetBoolValue(section.c_str(), key, val);
}

void SetValue(CSimpleIniA& ini, const std::string& section, const char* key, float val) {
    ini.SetDoubleValue(section.c_str(), key, static_cast<double>(val));
}

int GetValue(CSimpleIniA& ini, const std::string& section, const char* key, int val) {
    return ini.GetLongValue(section.c_str(), key, val);
}

std::string GetValue(CSimpleIniA& ini, const std::string& section, const char* key, const std::string& val) {
    return ini.GetValue(section.c_str(), key, val.c_str());
}

bool GetValue(CSimpleIniA& ini, const std::string& section, const char* key, bool val) {
    return ini.GetBoolValue(section.c_str(), key, val);
}

float GetValue(CSimpleIniA& ini, const std::string& section, const char* key, float val) {
    return static_cast<float>(ini.GetDoubleValue(section.c_str(), key, val));
}
