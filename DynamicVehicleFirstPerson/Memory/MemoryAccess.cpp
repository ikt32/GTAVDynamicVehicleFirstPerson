#include "MemoryAccess.hpp"

#include "../Util/Logger.hpp"
#include "../Util/Strings.hpp"
#include <inc/main.h>
#include <Windows.h>
#include <Psapi.h>
#include <vector>

namespace Memory {
    uintptr_t(*GetAddressOfEntity)(int entity) = nullptr;
    uintptr_t(*GetModelInfo)(unsigned int modelHash, int* index) = nullptr;

    float* timeScaleAddress = nullptr;
}

void Memory::Init() {
    auto addr = FindPattern("\x83\xF9\xFF\x74\x31\x4C\x8B\x0D\x00\x00\x00\x00\x44\x8B\xC1\x49\x8B\x41\x08",
        "xxxxxxxx????xxxxxxx");
    if (!addr) LOG(ERROR, "Couldn't find GetAddressOfEntity");
    GetAddressOfEntity = reinterpret_cast<uintptr_t(*)(int)>(addr);

    if (getGameVersion() < 58) {
        addr = FindPattern(
            "\x0F\xB7\x05\x00\x00\x00\x00"
            "\x45\x33\xC9\x4C\x8B\xDA\x66\x85\xC0"
            "\x0F\x84\x00\x00\x00\x00"
            "\x44\x0F\xB7\xC0\x33\xD2\x8B\xC1\x41\xF7\xF0\x48"
            "\x8B\x05\x00\x00\x00\x00"
            "\x4C\x8B\x14\xD0\xEB\x09\x41\x3B\x0A\x74\x54",
            "xxx????"
            "xxxxxxxxx"
            "xx????"
            "xxxxxxxxxxxx"
            "xx????"
            "xxxxxxxxxxx");

        if (!addr) {
            LOG(ERROR, "Couldn't find GetModelInfo");
        }
    }
    else {
        addr = FindPattern("\xEB\x09\x41\x3B\x0A\x74\x54", "xxxxxxx");
        if (!addr) {
            LOG(ERROR, "Couldn't find GetModelInfo (v58+)");
        }
        addr = addr - 0x2C;
    }

    GetModelInfo = reinterpret_cast<uintptr_t(*)(unsigned int modelHash, int* index)>(addr);

    // From ScriptHookVDotNet
    addr = FindPattern("\xF3\x0F\x11\x05\x00\x00\x00\x00\xF3\x0F\x10\x08\x0F\x2F\xC8\x73\x03\x0F\x28\xC1\x48\x83\xC0\x04\x49\x2B",
        "xxxx????xxxxxxxxxxxxxxxxxx");
    if (!addr) {
        LOG(ERROR, "Couldn't find TimeScaleAddress1");
    }
    else {
        auto timeScaleArrayAddress = (float*)(*(int*)(addr + 4) + addr + 8);
        if (timeScaleArrayAddress != nullptr)
            // SET_TIME_SCALE changes the 2nd element, so obtain the address of it
            timeScaleAddress = timeScaleArrayAddress + 1;
        else
            LOG(ERROR, "Couldn't find TimeScaleAddress2");
    }
}

uintptr_t Memory::FindPattern(const char* pattern, const char* mask) {
    MODULEINFO modInfo = { nullptr };
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

    const char* start_offset = reinterpret_cast<const char*>(modInfo.lpBaseOfDll);
    const uintptr_t size = static_cast<uintptr_t>(modInfo.SizeOfImage);

    intptr_t pos = 0;
    const uintptr_t searchLen = static_cast<uintptr_t>(strlen(mask) - 1);

    for (const char* retAddress = start_offset; retAddress < start_offset + size; retAddress++) {
        if (*retAddress == pattern[pos] || mask[pos] == '?') {
            if (mask[pos + 1] == '\0')
                return (reinterpret_cast<uintptr_t>(retAddress) - searchLen);
            pos++;
        }
        else {
            pos = 0;
        }
    }
    return 0;
}

uintptr_t Memory::FindPattern(const char* pattStr) {
    std::vector<std::string> bytesStr = StrUtil::Split(pattStr, ' ');

    MODULEINFO modInfo{};
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

    auto* start_offset = static_cast<uint8_t*>(modInfo.lpBaseOfDll);
    const auto size = static_cast<uintptr_t>(modInfo.SizeOfImage);

    uintptr_t pos = 0;
    const uintptr_t searchLen = bytesStr.size();
    std::vector<uint8_t> bytes;
    // Thanks Zolika for the performance improvement!
    for (const auto& str : bytesStr) {
        if (str == "??" || str == "?") bytes.push_back(0);
        else bytes.push_back(static_cast<uint8_t>(std::strtoul(str.c_str(), nullptr, 16)));
    }

    for (auto* retAddress = start_offset; retAddress < start_offset + size; retAddress++) {
        if (bytesStr[pos] == "??" || bytesStr[pos] == "?" ||
            *retAddress == bytes[pos]) {
            if (pos + 1 == bytesStr.size())
                return (reinterpret_cast<uintptr_t>(retAddress) - searchLen + 1);
            pos++;
        }
        else {
            pos = 0;
        }
    }
    return 0;
}

float Memory::GetTimeScale() {
    if (timeScaleAddress)
        return *timeScaleAddress;
    return 1.0f;
}
