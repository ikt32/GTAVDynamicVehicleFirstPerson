#pragma once
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_SUFFIX ""

namespace Constants {
    static const char* const ScriptName = "Dynamic Vehicle FPV";
    static const char* const NotificationPrefix = "~b~Dynamic Vehicle FPV~w~";
    static const char* const DisplayVersion = "v" STR(VERSION_MAJOR) "."  STR(VERSION_MINOR) "." STR(VERSION_PATCH) VERSION_SUFFIX;

    static const char* const iktFolder = "ikt";
    static const char* const ScriptFolder = "DynamicVehicleFirstPerson";
}
