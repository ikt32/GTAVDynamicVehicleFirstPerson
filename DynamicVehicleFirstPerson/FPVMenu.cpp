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
#include <algorithm>

namespace {
    // Keep in sync with EMountPoint
    std::vector<std::string> mountPointNames{
        "Vehicle",
        "Driver"
    };

    // See SHorizonLock::PitchMode
    const std::vector<std::string> PitchModeNames{
        "Horizon",
        "Vehicle",
        "Dynamic"
    };

    // On return false, an option is already created and the submenu may exit
    bool CreateCameraSubtitle(NativeMenu::Menu& mbCtx, CConfig* config) {
        if (config == nullptr) {
            mbCtx.Subtitle("~r~Error");
            mbCtx.Option("No active vehicle/configuration");
            return false;
        }
        if (config->CamIndex >= config->Mount.size()) {
            mbCtx.Subtitle("~r~Error");
            mbCtx.Option("~r~Error: CamIndex >= Mount.size()");
            return false;
        }
        mbCtx.Subtitle(std::format("{} - {}",
            config->Name,
            config->Mount[config->CamIndex].Name));
        return true;
    }
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
            else if (context.ActiveConfig()->Mount.empty()) {
                mbCtx.Option("~c~Corrupt config",
                    { std::format("Config '{}' contains 0 cameras, but should contain at least 1.",
                        context.ActiveConfig()->Name),
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

                mbCtx.OptionPlus(std::format("Select camera <{}: {}>", cfg->Mount[cfg->CamIndex].Name, cfg->CamIndex + 1),
                    FormatCameraInfo(*cfg, cfg->CamIndex),
                    nullptr, onRight, onLeft, "Camera info",
                    { "Switch cameras for different viewpoints." });

                mbCtx.MenuOption("Camera settings", "cam.settings.menu",
                    { "Change active camera parameters." });

                mbCtx.MenuOption("Manage cameras", "cam.manage.menu",
                    { "View, add or remove cameras from this config." });
            }

            mbCtx.MenuOption("Manage configs", "cfg.manage.menu",
                { "Create a new config or view other configs." });

            if (FPV::GetSettings().Debug.Enable) {
                mbCtx.MenuOption("Debug", "debug.menu",
                    { "Yeah." });
            }
        });

    submenus.emplace_back("cfg.manage.menu",
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
                    { "Press enter/select to create a new config "
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

    submenus.emplace_back("cam.settings.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Camera settings");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }

            mbCtx.MenuOption("Sensitivity settings", "sensitivity.menu",
                { "Adjust mouse/controller sensitivity.",
                  "These options apply to all cameras in this config." });

            if (config->CamIndex >= config->Mount.size()) {
                mbCtx.Option("~r~Error: CamIndex >= Mount.size()");
                return;
            }
            CConfig::SCameraSettings& cam = config->Mount[config->CamIndex];

            int mountInt = static_cast<int>(cam.MountPoint);
            if (mbCtx.StringArray("Attach to", mountPointNames, mountInt,
                { "Mounting to the vehicle is pretty static and predictable.",
                  "Mounting to the driver transmit all their animations." })) {
                cam.MountPoint = static_cast<CConfig::EMountPoint>(mountInt);

                // it'll re-acquire next tick with the correct position.
                context.Cancel();
            }

            mbCtx.FloatOptionCb("Field of view", cam.FOV, 1.0f, 120.0f, 0.5f, FPV::GetKbEntryFloat,
                { "In degrees." });

            mbCtx.FloatOptionCb("Height offset", cam.OffsetHeight, -2.0f, 2.0f, 0.01f, FPV::GetKbEntryFloat,
                { "Distance in meters." });

            mbCtx.FloatOptionCb("Forward offset", cam.OffsetForward, -2.0f, 2.0f, 0.01f, FPV::GetKbEntryFloat,
                { "Distance in meters." });

            mbCtx.FloatOptionCb("Side offset", cam.OffsetSide, -2.0f, 2.0f, 0.01f, FPV::GetKbEntryFloat,
                { "Distance in meters." });

            mbCtx.FloatOptionCb("Pitch offset", cam.Pitch, -20.0f, 20.0f, 0.1f, FPV::GetKbEntryFloat,
                { "In degrees." });

            mbCtx.MenuOption("Leaning options", "lean.menu",
                { "Modify how the camera moves when looking back, for better visibility." });

            mbCtx.MenuOption("Camera inertia options", "inertia.menu",
                { "Modify camera inertia and movement." });

            mbCtx.MenuOption("Horizon lock options", "horizon.menu",
                { "Modify horizon lock." });

            mbCtx.MenuOption("Depth of field options", "dof.menu",
                { "Modify depth of field effects." });
        });

    submenus.emplace_back("sensitivity.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Sensitivity");
            CConfig* config = context.ActiveConfig();
            mbCtx.Subtitle(std::format("Config: {}", config ? config->Name : "None"));
            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            mbCtx.FloatOptionCb("Controller smoothing", config->Look.LookTime, 0.0f, 0.5f, 0.000001f, GetKbEntryFloat,
                { "How smooth the camera moves.", "Press enter to enter a value manually. Range: 0.0 to 0.5." });

            mbCtx.FloatOptionCb("Mouse sensitivity", config->Look.MouseSensitivity, 0.05f, 2.0f, 0.05f, GetKbEntryFloat);

            mbCtx.FloatOptionCb("Mouse smoothing", config->Look.MouseLookTime, 0.0f, 0.5f, 0.000001f, GetKbEntryFloat,
                { "How smooth the camera moves.", "Press enter to enter a value manually. Range: 0.0 to 0.5." });

            mbCtx.IntOptionCb("Mouse center timeout", config->Look.MouseCenterTimeout, 0, 120000, 500, FPV::GetKbEntryInt,
                { "Milliseconds before centering the camera after looking with the mouse." });
        });

    submenus.emplace_back("lean.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Leaning");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SCameraSettings& cam = config->Mount[config->CamIndex];

            mbCtx.FloatOptionCb("Center distance", cam.Lean.CenterDist, -2.0f, 2.0f, 0.01f, GetKbEntryFloat,
                { "Distance in meters to lean over to the center when looking back." });

            mbCtx.FloatOptionCb("Forward distance", cam.Lean.ForwardDist, -2.0f, 2.0f, 0.01f, GetKbEntryFloat,
                { "Distance in meters to lean forward when looking back/sideways." });

            mbCtx.FloatOptionCb("Up distance", cam.Lean.UpDist, -2.0f, 2.0f, 0.01f, GetKbEntryFloat,
                { "Distance in meters to peek up when looking back." });
        });

    submenus.emplace_back("inertia.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Inertia & movement");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SMovement& movement = config->Mount[config->CamIndex].Movement;

            if (mbCtx.BoolOption("Enable inertia & movement", movement.Follow,
                { "Enable to allow the camera to rotate and move around in response to physics.",
                  "When disabled, camera is rigidly mounted to vehicle or driver." })) {
                context.Cancel();
            }

            mbCtx.MenuOption("Rotation", "inertia.rot.menu",
                { "Options for how vehicle movement affects the camera.",
                  "Affects camera yaw." });

            mbCtx.MenuOption("Inertia: Longitudinal", "inertia.long.menu",
                { "Options for how acceleration affect the camera.",
                  "Affects camera pitch and forward/backward movement." });

            mbCtx.MenuOption("Inertia: Lateral", "inertia.lat.menu",
                { "Options for how sideways acceleration affects the camera.",
                  "Affects camera left/right movement." });

            mbCtx.MenuOption("Inertia: Vertical", "inertia.vert.menu",
                { "Options for how vertical acceleration affects the camera.",
                  "Affects camera up/down movement." });

            mbCtx.FloatOptionCb("Movement roughness", movement.Roughness, 0.0f, 10.0f, 0.5f, GetKbEntryFloat,
                { "How rough the camera movement is, from inertia effects.",
                  "Larger values increase roughness, causing smaller bumps to be more noticeable.",
                  "Smaller values increase smoothness, but may cause the movement to be less responsive." });

            bool bumpTrigger = mbCtx.FloatOptionCb("Bump severity", movement.Bump, 0.0f, 10.0f, 0.5f, GetKbEntryFloat,
                { "How much the vehicle itself responds to bumps.",
                  "Recommended to set this to 0, as the vehicle body moving around is just a visual effect." });
            if (bumpTrigger) {
                VEHICLE::SET_CAR_HIGH_SPEED_BUMP_SEVERITY_MULTIPLIER(movement.Bump);
            }
        });

    submenus.emplace_back("inertia.rot.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Rotation");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SMovement& movement = config->Mount[config->CamIndex].Movement;

            mbCtx.FloatOption("Direction multiplier", movement.RotationDirectionMult, 0.0f, 4.0f, 0.01f,
                { "How much the direction of travel affects the camera." });

            mbCtx.FloatOption("Rotation: rotation", movement.RotationRotationMult, 0.0f, 4.0f, 0.01f,
                { "How much the rotation speed affects the camera." });

            mbCtx.FloatOption("Rotation: max angle", movement.RotationMaxAngle, 0.0f, 90.0f, 1.0f,
                { "To how many degrees camera movement is capped." });
        });

    submenus.emplace_back("inertia.long.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Longitudinal inertia");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SMovement& movement = config->Mount[config->CamIndex].Movement;

            mbCtx.FloatOption("Minimum force", movement.LongDeadzone, 0.0f, 2.0f, 0.01f,
                { "How hard the car should accelerate or decelerate for the camera to start moving.",
                  "Unit in Gs." });

            mbCtx.FloatOption("Forward scale", movement.LongForwardMult, 0.0f, 2.0f, 0.01f,
                { "How much the camera moves forwards when decelerating.",
                  "A scale of 1.0 makes the camera move 1 meter at 1G deceleration.",
                  "A scale of 0.1 makes the camera move 10 centimeters at 1G deceleration.",
                  "0.0 disables forward movement." });

            mbCtx.FloatOption("Backward scale", movement.LongBackwardMult, 0.0f, 2.0f, 0.01f,
                { "How much the camera moves backwards when accelerating.",
                  "A scale of 1.0 makes the camera move 1 meter at 1G acceleration.",
                  "A scale of 0.1 makes the camera move 10 centimeters at 1G acceleration.",
                  "0.0 disables backward movement." });

            mbCtx.FloatOption("Forward limit", movement.LongForwardLimit, 0.0f, 1.0f, 0.01f,
                { "How much the camera may move forwards during deceleration.",
                  "Unit in meter." });

            mbCtx.FloatOption("Backward limit", movement.LongBackwardLimit, 0.0f, 1.0f, 0.01f,
                { "How much the camera may move backwards during acceleration.",
                  "Unit in meter." });

            mbCtx.FloatOptionCb("Pitch: Minimum force", movement.PitchDeadzone, 0.0f, 2.0f, 0.01f, GetKbEntryFloat,
                { "How much the car should accelerate or decelerate for the camera to start moving.",
                  "Unit in Gs." });

            mbCtx.FloatOptionCb("Pitch: Up scale", movement.PitchUpMult, 0.0f, 90.0f, 0.01f, GetKbEntryFloat,
                { "How much the camera pitches up during acceleration.",
                  "A scale of 5.0 makes the camera pitch up 5 degrees at 1G acceleration.",
                  "0.0 disables pitch-up on acceleration." });

            mbCtx.FloatOptionCb("Pitch: Down scale", movement.PitchDownMult, 0.0f, 90.0f, 0.01f, GetKbEntryFloat,
                { "How much the camera pitches down during deceleration.",
                  "A scale of 5.0 makes the camera pitch down 5 degrees at 1G deceleration.",
                  "0.0 disables pitch-down on deceleration." });

            mbCtx.FloatOptionCb("Pitch: Up limit", movement.PitchUpMaxAngle, 0.0f, 90.0f, 0.5f, GetKbEntryFloat,
                { "How much the camera may pitch up during acceleration.",
                  "Unit in degrees." });

            mbCtx.FloatOptionCb("Pitch: Down limit", movement.PitchDownMaxAngle, 0.0f, 90.0f, 0.5f, GetKbEntryFloat,
                { "How much the camera may pitch down during deceleration.",
                  "Unit in degrees." });
        });

    submenus.emplace_back("inertia.lat.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Lateral inertia");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SMovement& movement = config->Mount[config->CamIndex].Movement;

            mbCtx.FloatOption("Minimum force", movement.LatDeadzone, 0.0f, 2.0f, 0.01f,
                { "How hard the car should turn or accelerate sideways for the camera to start moving.",
                  "Unit in Gs." });

            mbCtx.FloatOption("Scale", movement.LatMult, -2.0f, 2.0f, 0.01f,
                { "How much the camera moves left or right.",
                  "A scale of 1.0 makes the camera move 1 meter at 1G.",
                  "A scale of 0.1 makes the camera move 10 centimeters at 1G.",
                  "Negative values make the camera move \"against\" the force.",
                  "0.0 disables lateral movement." });

            mbCtx.FloatOption("Limit", movement.LatLimit, 0.0f, 1.0f, 0.01f,
                { "How much the camera may move left or right.",
                  "Unit in meter." });
        });

    submenus.emplace_back("inertia.vert.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Vertical inertia");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SMovement& movement = config->Mount[config->CamIndex].Movement;

            mbCtx.FloatOption("Minimum force", movement.VertDeadzone, 0.0f, 2.0f, 0.01f,
                { "How hard the car goes up or down for the camera to start moving.",
                  "Unit in Gs." });

            mbCtx.FloatOption("Up scale", movement.VertUpMult, 0.0f, 2.0f, 0.01f,
                { "How much the camera moves up when falling.",
                  "A scale of 1.0 makes the camera move 1 meter at 1G.",
                  "A scale of 0.1 makes the camera move 10 centimeters at 1G.",
                  "0.0 disables up movement." });

            mbCtx.FloatOption("Down scale", movement.VertDownMult, 0.0f, 2.0f, 0.01f,
                { "How much the camera moves down when \"pushed down\".",
                  "A scale of 1.0 makes the camera move 1 meter at 1G.",
                  "A scale of 0.1 makes the camera move 10 centimeters at 1G.",
                  "0.0 disables down movement." });

            mbCtx.FloatOption("Up limit", movement.VertUpLimit, 0.0f, 1.0f, 0.01f,
                { "How much the camera may move up.",
                  "Unit in meter." });

            mbCtx.FloatOption("Down limit", movement.VertDownLimit, 0.0f, 1.0f, 0.01f,
                { "How much the camera may move down.",
                  "Unit in meter." });
        });

    submenus.emplace_back("horizon.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Horizon lock");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SHorizonLock& horLck = config->Mount[config->CamIndex].HorizonLock;

            mbCtx.BoolOption("Lock to horizon", horLck.Lock,
                { "Lock the pitch and roll to the horizon." });

            mbCtx.FloatOptionCb("Pitch limit", horLck.PitchLim, 0.0f, 90.0f, 1.0f, GetKbEntryFloat,
                { "How much the pitch may differ between the camera and vehicle." });

            mbCtx.FloatOptionCb("Roll limit", horLck.RollLim, 0.0f, 180.0f, 1.0f, GetKbEntryFloat,
                { "How much the roll may differ between the camera and vehicle." });

            mbCtx.StringArray("Lock pitch to", PitchModeNames, horLck.PitchMode,
                { "Lock pitch with horizon, car or center on vehicle dynamically." });

            mbCtx.FloatOptionCb("Pitch center speed", horLck.CenterSpeed, 0.1f, 10.0f, 0.1f, GetKbEntryFloat,
                { "How quickly the camera centers on the vehicle pitch.",
                    "Low value: Slowly centers onto the vehicle.",
                    "High value: Quickly centers onto the vehicle." });
        });

    submenus.emplace_back("dof.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Depth of field");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }
            CConfig::SDoF& dof = config->Mount[config->CamIndex].DoF;

            mbCtx.BoolOption("Enable", dof.Enable,
                { "Enable or disable dynamic depth of field.",
                  "Unfocuses the car and very far distances at high speed, for extra sense of speed.",
                  "Very High post-processing required! May have significant performance impact.",
                  "All options below can be edited precisely, press <Enter> to enter a number." });

            mbCtx.FloatOptionCb("TargetSpeedMinDoF", dof.TargetSpeedMinDoF,
                0.0f, 200.0f, 1.0f, GetKbEntryFloat,
                { "Speed at which unfocusing starts, in m/s.",
                  std::format("({:.0f} kph, {:.0f} mph)",
                      dof.TargetSpeedMinDoF * 3.6f,
                      dof.TargetSpeedMinDoF * 2.23694f) });

            mbCtx.FloatOptionCb("TargetSpeedMaxDoF", dof.TargetSpeedMaxDoF,
                0.0f, 400.0f, 1.0f, GetKbEntryFloat,
                { "Speed at which unfocusing is largest, in m/s.",
                  std::format("({:.0f} kph, {:.0f} mph)",
                      dof.TargetSpeedMaxDoF * 3.6f,
                      dof.TargetSpeedMaxDoF * 2.23694f) });

            mbCtx.FloatOptionCb("NearOutFocusMinSpeedDist", dof.NearOutFocusMinSpeedDist,
                0.0f, 10.0f, 0.01f, GetKbEntryFloat,
                { "Distance of the plane that's out of focus, when traveling at or below 'TargetSpeedMinDoF'.",
                  "In meters.",
                  "Default: 0.00: nothing is blurred when going slow." });

            mbCtx.FloatOptionCb("NearOutFocusMaxSpeedDist", dof.NearOutFocusMaxSpeedDist,
                0.0f, 10.0f, 0.01f, GetKbEntryFloat,
                { "Distance of the plane that's out of focus, when traveling at or above 'TargetSpeedMaxDoF'.",
                  "In meters.",
                  "Default: 0.50: everything closer than 0.5 meters (1.6 feet) to the camera is blurred when going fast." });

            mbCtx.FloatOptionCb("NearInFocusMinSpeedDist", dof.NearInFocusMinSpeedDist,
                0.1f, 100.0f, 0.1f, GetKbEntryFloat,
                { "Distance of the plane that's in focus (when things stop being blurry), when traveling at or below 'TargetSpeedMinDoF'.",
                  "In meters.",
                  "Default: 0.10: Only objects closer than 0.1 meters (4 inches) to the camera are blurred when going slow.",
                  "Must be higher than 'NearOutFocusMinSpeedDist'." });

            mbCtx.FloatOptionCb("NearInFocusMaxSpeedDist", dof.NearInFocusMaxSpeedDist,
                1.0f, 100.0f, 1.0f, GetKbEntryFloat,
                { "Distance of the plane that's in focus (when things stop being blurry), when traveling at or above 'TargetSpeedMaxDoF'.",
                  "In meters.",
                  "Default: 20.0, where everything closer than 20 meters (65 feet) is blurred when going fast.",
                  "Must be much higher than 'NearOutFocusMaxSpeedDist'." });

            mbCtx.FloatOptionCb("FarInFocusMinSpeedDist", dof.FarInFocusMinSpeedDist,
                100.0f, 100000.0f, 1.0f, GetKbEntryFloat,
                { "Distance of the plane that's in focus (when things start being blurry), when traveling at or below 'TargetSpeedMinDoF'.",
                  "In meters.",
                  "Default: 100000: Practically infinite, no distant blur." });

            mbCtx.FloatOptionCb("FarInFocusMaxSpeedDist", dof.FarInFocusMaxSpeedDist,
                100.0f, 100000.0f, 1.0f, GetKbEntryFloat,
                { "Distance of the plane that's in focus (when things stop being blurry), when traveling at or above 'TargetSpeedMaxDoF'.",
                  "In meters.",
                  "Default: 2000.0: Everything farther than 2 km (1.2 miles) starts to get blurred when going fast." });

            mbCtx.FloatOptionCb("FarOutFocusMinSpeedDist", dof.FarOutFocusMinSpeedDist,
                100.0f, 100000.0f, 0.01f, GetKbEntryFloat,
                { "Distance of the plane that's out of focus, when traveling at or below 'TargetSpeedMinDoF'.",
                  "In meters.",
                  "Default: 100000: Practically infinite, no distant blur.",
                  "Must be higher or equal to 'FarInFocusMinSpeedDist'." });

            mbCtx.FloatOptionCb("FarOutFocusMaxSpeedDist", dof.FarOutFocusMaxSpeedDist,
                100.0f, 100000.0f, 0.01f, GetKbEntryFloat,
                { "Distance of the plane that's out of focus, when traveling at or above 'TargetSpeedMaxDoF'.",
                  "In meters.",
                  "Default: 10000: Everything farther than 10 km (6.2 miles) is as blurred can be, when going fast.",
                  "Must be higher than 'FarInFocusMaxSpeedDist'." });

            mbCtx.FloatOptionCb("TargetAccelMinDoF", dof.TargetAccelMinDoF,
                0.0f, 200.0f, 0.05f, GetKbEntryFloat,
                { "Acceleration where unfocusing is reduced, in m/s^2.",
                  std::format("({:.2f} G)", dof.TargetAccelMinDoF / 9.81f),
                  "Default: 0.5G, to reduce blur when not accelerating or coasting." });

            mbCtx.FloatOptionCb("TargetAccelMaxDoF", dof.TargetAccelMaxDoF,
                0.0f, 200.0f, 0.05f, GetKbEntryFloat,
                { "Acceleration where unfocusing is increased, in m/s^2.",
                  std::format("({:.2f} G)", dof.TargetAccelMaxDoF / 9.81f),
                  "Default: 1.0G, at which blur (for that speed) is maximized." });

            mbCtx.FloatOptionCb("TargetAccelMinDoFMod", dof.TargetAccelMinDoFMod,
                0.0f, 10.0f, 0.01f, GetKbEntryFloat,
                { "Modifier for blur reduction when at or below 'TargetAccelMinDoF' acceleration.",
                  "Default: 0.1, at low acceleration the near blur is moved closer to the camera, unblurring the dashboard and wheel." });

            mbCtx.FloatOptionCb("TargetAccelMaxDoFMod", dof.TargetAccelMaxDoFMod,
                0.0f, 10.0f, 0.01f, GetKbEntryFloat,
                { "Modifier for blur reduction when at or above 'TargetAccelMaxDoF' acceleration.",
                  "Default: 1.0, at high acceleration the near blur is as far forward as decided by the speed." });
        });

    submenus.emplace_back("cam.manage.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Camera management");
            CConfig* config = context.ActiveConfig();
            if (!CreateCameraSubtitle(mbCtx, config)) {
                return;
            }

            if (config->Mount.size() < 10) {
                if (mbCtx.Option("Add camera",
                    { "Add a new camera to this config." })) {
                    AddCamera(*config, nullptr);
                    return;
                }
            }
            else {
                mbCtx.Option("~c~Cameras full",
                    { "No more than 10 cameras can be added." });
            }

            // Swap after we're done showing the list.
            bool anyQueued = false;
            int queueMoveDown = -1;
            int queueMoveUp = -1;

            for (int i = 0; i < config->Mount.size(); ++i) {
                auto cameraName = config->Mount[i].Name;
                auto optionName = std::format("{} {}",
                    config->CamIndex == i ? "> " : "",
                    cameraName);

                auto onLeft = [&]() {
                    if (config->Mount[i].Order > 0 &&
                        !anyQueued) {
                        queueMoveUp = i;
                        anyQueued = true;
                    }
                };
                auto onRight = [&]() {
                    if (config->Mount[i].Order < config->Mount.size() - 1 &&
                        !anyQueued) {
                        queueMoveDown = i;
                        anyQueued = true;
                    }
                };

                bool selected;
                bool triggered = mbCtx.OptionPlus(optionName,
                    {}, &selected, onRight, onLeft, "",
                    { "Press enter to copy this camera, or to remove it.",
                      "Press left to increase priority, right to decrease it." });

                if (selected) {
                    mbCtx.OptionPlusPlus(FormatCameraInfo(*config, i), cameraName);
                }

                if (triggered) {
                    UI::ShowHelpText(
                        "Enter 'copy' to copy camera.~n~"
                        "Enter 'delete' to delete camera.~n~"
                        "(Both without quotes)");

                    std::string choice = GetKbEntryString("copy");
                    if (choice == "copy") {
                        AddCamera(*config, &config->Mount[i]);
                        return;
                    }
                    else if (choice == "delete") {
                        DeleteCamera(*config, config->Mount[i]);
                        return;
                    }
                    else {
                        UI::Notify("No valid choice entered, cancelled camera copy/delete.");
                    }
                }
            }

            if (queueMoveDown != -1) {
                int i = queueMoveDown;
                ++config->Mount[i].Order;
                --config->Mount[i + 1].Order;
                std::swap(config->Mount[i], config->Mount[i + 1]);
                mbCtx.NextOption();
            }
            else if (queueMoveUp != -1) {
                int i = queueMoveUp;
                ++config->Mount[i - 1].Order;
                --config->Mount[i].Order;
                std::swap(config->Mount[i - 1], config->Mount[i]);
                mbCtx.PreviousOption();
            }
        });

    submenus.emplace_back("debug.menu",
        [](NativeMenu::Menu& mbCtx, CFPVScript& context) {
            mbCtx.Title("Debug");
            mbCtx.Subtitle("(:");

            if (mbCtx.BoolOption("Disable FPV head hiding", FPV::GetSettings().Debug.DisableRemoveHead,
                { "Disables hiding player head with CamxxCore's DismembermentASI.asi present." })) {
                context.HideHead(!FPV::GetSettings().Debug.DisableRemoveHead);
            }

            mbCtx.BoolOption("Disable FPV props removal", FPV::GetSettings().Debug.DisableRemoveProps,
                { "Disables removing temporarily removing props (head, eyes) from the player's head.",
                  "Toggle when not in custom FPV, otherwise props may be lost." });

            mbCtx.BoolOption("Override FPV near clip", FPV::GetSettings().Debug.NearClip.Override,
                { "Overrides near clip to always use the value below." });

            mbCtx.FloatOptionCb("FPV near clip", FPV::GetSettings().Debug.NearClip.Distance, 0.0f, 10.0f, 0.05f, GetKbEntryFloat);

            mbCtx.BoolOption("Override DoF", FPV::GetSettings().Debug.DoF.Override,
                { "When DoF is enabled for the FPV camera, temporarily override it with the values below.",
                  "May be useful to tweak distances for different vehicles." });
            mbCtx.FloatOptionCb("NearOutFocus", FPV::GetSettings().Debug.DoF.NearOutFocus, 0.0f, 100000.0f, 0.05f, GetKbEntryFloat);
            mbCtx.FloatOptionCb("NearInFocus", FPV::GetSettings().Debug.DoF.NearInFocus, 0.0f, 100000.0f, 0.05f, GetKbEntryFloat);
            mbCtx.FloatOptionCb("FarInFocus", FPV::GetSettings().Debug.DoF.FarInFocus, 0.0f, 100000.0f, 1.00f, GetKbEntryFloat);
            mbCtx.FloatOptionCb("FarOutFocus", FPV::GetSettings().Debug.DoF.FarOutFocus, 0.0f, 100000.0f, 1.00f, GetKbEntryFloat);
        });

    return submenus;
}
