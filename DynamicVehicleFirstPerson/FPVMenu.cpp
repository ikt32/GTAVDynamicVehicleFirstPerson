#include "ScriptMenu.hpp"
#include "Constants.hpp"
#include "FPVMenuUtils.hpp"
#include "FPVScript.hpp"
#include "Script.hpp"

#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/ScriptUtils.hpp"
#include "Util/UI.hpp"

#include <inc/natives.h>

namespace {
    // Stateful menu stuff, if required
}

std::vector<CScriptMenu<CFPVScript>::CSubmenu> FPV::BuildMenu() {
    std::vector<CScriptMenu<CFPVScript>::CSubmenu> submenus;
    submenus.emplace_back("mainmenu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title(Constants::ScriptName);
            mbCtx.Subtitle(std::string("~b~") + Constants::DisplayVersion);

            mbCtx.BoolOption("Enable", FPV::GetSettings().Main.Enable,
                { "Enable or disable the entire script." });

            if (context.ActiveConfig() == nullptr) {
                mbCtx.Option("~c~No active config",
                    { "No config is active.",
                      "Camera options unavailable." });
            }
            else {
                CConfig* cfg = context.ActiveConfig();
                mbCtx.OptionPlus("Config info", FPV::FormatConfigInfo(*cfg));

                auto onRight = [&]() {
                    if (cfg->CamIndex < cfg->Mount.size() - 1) {
                        ++cfg->CamIndex;
                    }
                };
                auto onLeft = [&]() {
                    if (cfg->CamIndex > 0) {
                        --cfg->CamIndex;
                    }
                };

                mbCtx.OptionPlus("Select camera",
                    FormatCameraInfo(*cfg),
                    nullptr, onRight, onLeft, "Camera info",
                    { "Switch cameras for different viewpoints." });

                mbCtx.MenuOption("Manage cameras", "cammanagemenu",
                    { "No config is active, camera management unavailable." });
            }

            mbCtx.MenuOption("Manage configs", "cfgmanagemenu",
                { "Create a new config or view other configs." });
        });

    submenus.emplace_back("cfgmanagemenu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Configs");
            mbCtx.Subtitle("");

            Vehicle vehicle = context.GetVehicle();
            if (!Util::VehicleAvailable(vehicle, PLAYER::PLAYER_PED_ID())) {
                mbCtx.Option("~c~Create config...",
                    { "This is only available while in a vehicle." });
                return;
            }
            else if (context.ActiveConfig() == nullptr) {
                mbCtx.Option("~c~Create config...",
                    { "~r~Base config is null. This should never happen." });
                return;
            }

            if (mbCtx.Option("Create config...",
                { "Create a new configuration file.",
                    "Changes made within a configuration are saved to that configuration only.",
                    "Multiple configs with the same ID (Vehicle or Vehicle and Plate) are "
                    "not supported, script picks whatever comes first alphabetically." })) {
                CreateConfig(*context.ActiveConfig(), vehicle);
            }

            if (FPV::GetConfigs().empty()) {
                mbCtx.Option("No saved configs");
            }

            for (const auto& config : FPV::GetConfigs()) {
                bool selected;
                bool triggered = mbCtx.OptionPlus(config.Name, {}, &selected, nullptr, nullptr, "",
                    { "Press enter/select to create a new config"
                        "based on currently selected config." });

                if (selected) {
                    mbCtx.OptionPlusPlus(FPV::FormatConfigInfo(config), config.Name);
                }

                if (triggered) {
                    CConfig newConfig = config;
                    CreateConfig(newConfig, vehicle);
                }
            }
        });

    return submenus;
}
