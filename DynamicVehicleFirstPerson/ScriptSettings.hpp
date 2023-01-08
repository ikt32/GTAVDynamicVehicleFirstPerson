#pragma once
#include <string>

class CScriptSettings
{
public:
    CScriptSettings(std::string settingsFile);

    void Load();
    void Save();

    struct {
        bool Enable = true;
    } Main;

    struct {
        bool Enable = false;

        bool DisableRemoveHead = false;
        bool DisableRemoveProps = false;

        struct {
            bool Override = false;
            float Distance = 0.05f;
        } NearClip;

        struct {
            bool Override = false;
            float NearOutFocus = 0.00f;
            float NearInFocus = 0.20f;
            float FarInFocus = 5000.0f;
            float FarOutFocus = 100000.0f;
        } DoF;
    } Debug;

private:
    std::string mSettingsFile;
};
