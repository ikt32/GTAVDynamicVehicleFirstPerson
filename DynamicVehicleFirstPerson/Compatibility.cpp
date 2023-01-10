#include "Compatibility.hpp"

#include "Util/Logger.hpp"
#include <Windows.h>

namespace {
    HMODULE DismembermentModule = nullptr;
    void(*Dismemberment_AddBoneDraw)(int handle, int start, int end) = nullptr;
    void(*Dismemberment_RemoveBoneDraw)(int handle) = nullptr;

    HMODULE MTModule = nullptr;
    bool(*MT_LookingLeft)() = nullptr;
    bool(*MT_LookingRight)() = nullptr;
    bool(*MT_LookingBack)() = nullptr;
}

namespace Dismemberment {
    void Setup();
    void Cleanup();
}

namespace MT {
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
    MT::Setup();
}

void Compatibility::Cleanup() {
    Dismemberment::Cleanup();
    MT::Cleanup();
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

void MT::Setup() {
    LOG(INFO, "[Compat] Setting up Manual Transmission");
    MTModule = GetModuleHandle("Gears.asi");
    if (!MTModule) {
        LOG(INFO, "[Compat] Gears.asi not found");
        return;
    }

    MT_LookingLeft = CheckAddr<bool(*)()>(MTModule, "MT_LookingLeft");
    MT_LookingRight = CheckAddr<bool(*)()>(MTModule, "MT_LookingRight");
    MT_LookingBack = CheckAddr<bool(*)()>(MTModule, "MT_LookingBack");
}

void MT::Cleanup() {
    MTModule = nullptr;
    MT_LookingLeft = nullptr;
    MT_LookingRight = nullptr;
    MT_LookingBack = nullptr;
}

bool MT::Available() {
    return MT_LookingLeft != nullptr &&
        MT_LookingRight != nullptr &&
        MT_LookingBack != nullptr;
}

bool MT::LookingLeft() {
    if (MT_LookingLeft)
        return MT_LookingLeft();
    return false;
}

bool MT::LookingRight() {
    if (MT_LookingRight)
        return MT_LookingRight();
    return false;
}

bool MT::LookingBack() {
    if (MT_LookingBack)
        return MT_LookingBack();
    return false;
}
