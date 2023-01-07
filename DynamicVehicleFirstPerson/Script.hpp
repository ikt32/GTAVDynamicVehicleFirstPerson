#pragma once
#include "FPVScript.hpp"
#include "ScriptMenu.hpp"

namespace FPV {
    void ScriptMain();
    std::vector<CScriptMenu<CFPVScript>::CSubmenu> BuildMenu();
}
