#include "Compatibility.hpp"

#include "Util/Logger.hpp"
#include <Windows.h>

namespace {
    HMODULE DismembermentModule = nullptr;
    void(*Dismemberment_AddBoneDraw)(int handle, int start, int end) = nullptr;
    void(*Dismemberment_RemoveBoneDraw)(int handle) = nullptr;
}

namespace Dismemberment {
    void Setup();
    void Cleanup();
}

template <typename T>
T CheckAddr(HMODULE lib, const std::string& funcName) {
    FARPROC func = GetProcAddress(lib, funcName.c_str());
    if (!func) {
        LOG(ERROR, "[Compat] Couldn't get function [{}]", funcName);
        return nullptr;
    }
    LOG(DEBUG, "[Compat] Found function [{}]", funcName);
    return reinterpret_cast<T>(func);
}

void Compatibility::Setup() {
    Dismemberment::Setup();
}

void Compatibility::Cleanup() {
    Dismemberment::Cleanup();
}

void Dismemberment::Setup() {
    LOG(INFO, "[Compat] Setting up DismembermentASI");
    DismembermentModule = GetModuleHandle("DismembermentASI.asi");
    if (!DismembermentModule) {
        LOG(INFO, "[Compat] DismembermentASI.asi not found");
        return;
    }

    Dismemberment_AddBoneDraw = CheckAddr<void(*)(int, int, int)>(DismembermentModule, "AddBoneDraw");
    Dismemberment_RemoveBoneDraw = CheckAddr<void(*)(int)>(DismembermentModule, "RemoveBoneDraw");
}

void Dismemberment::Cleanup() {
    DismembermentModule = nullptr;
    Dismemberment_AddBoneDraw = nullptr;
    Dismemberment_RemoveBoneDraw = nullptr;
}

bool Dismemberment::Available() {
    return Dismemberment_AddBoneDraw != nullptr &&
        Dismemberment_RemoveBoneDraw != nullptr;
}

void Dismemberment::AddBoneDraw(int handle, int start, int end) {
    if (Dismemberment_AddBoneDraw)
        Dismemberment_AddBoneDraw(handle, start, end);
}

void Dismemberment::RemoveBoneDraw(int handle) {
    if (Dismemberment_RemoveBoneDraw)
        Dismemberment_RemoveBoneDraw(handle);
}
