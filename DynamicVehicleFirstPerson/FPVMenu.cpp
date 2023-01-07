#include "ScriptMenu.hpp"
#include "Constants.hpp"
#include "FPVScript.hpp"
#include "Script.hpp"

#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/UI.hpp"

#include <inc/main.h>

namespace {
    // TODO
}

std::vector<CScriptMenu<CFPVScript>::CSubmenu> FPV::BuildMenu() {
    std::vector<CScriptMenu<CFPVScript>::CSubmenu> submenus;
    submenus.emplace_back("mainmenu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title(Constants::ScriptName);
            mbCtx.Subtitle(std::string("~b~") + Constants::DisplayVersion);

            // TODO

            int nothing = 0;
            mbCtx.StringArray("Version", { Constants::DisplayVersion }, nothing,
                { "Thanks for checking out this menu!", "Author: ikt" });
        });

    return submenus;
}
