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
        bool DisableRemoveHead = false;
        bool DisableRemoveProps = false;
    } Debug;

private:
    std::string mSettingsFile;
};
