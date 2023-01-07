#include "Script.hpp"
#include "ScriptMenu.hpp"
#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/UI.hpp"

#include <inc/main.h>

namespace {
    std::shared_ptr<CFPVScript> coreScript;
    std::unique_ptr<CScriptMenu<CFPVScript>> scriptMenu;

    bool initialized = false;
}

namespace FPV {
    void scriptInit();
    void scriptTick();
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
    const auto settingsMenuPath = Paths::GetModPath() / "settings_menu.ini";

    coreScript = std::make_shared<CFPVScript>();

    // The menu being initialized. Note the passed settings,
    // the onInit and onExit lambdas and finally BuildMenu being called.
    scriptMenu = std::make_unique<CScriptMenu<CFPVScript>>(settingsMenuPath.string(),
        []() {
            // onInit
        },
        []() {
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
