#include "VehicleExtensions.hpp"

#include "MemoryAccess.hpp"
#include "../Util/Logger.hpp"

namespace {
    int hoverTransformRatioOffset = 0;
    int rpmOffset = 0;
    int wheelsContainerOffset = 0;
    int wheelCountOffset = 0;

    int wheelSuspensionCompressionOffset = 0;
    int wheelMatTypeOffset = 0;
}

void VehicleExtensions::Init() {
    // alloc8or
    auto addr = Memory::FindPattern("F3 0F 11 B3 ? ? ? ? 44 88 ? ? ? ? ? 48 85 C9");
    hoverTransformRatioOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(hoverTransformRatioOffset == 0 ? WARN : DEBUG, "[VExt] Hover Transform Active offset: 0x{:03X}", hoverTransformRatioOffset);

    addr = Memory::FindPattern("76 03 0F 28 F0 F3 44 0F 10 93");
    rpmOffset = addr == 0 ? 0 : *(int*)(addr + 10);
    LOG(rpmOffset == 0 ? WARN : DEBUG, "[VExt] RPM offset: 0x{:03X}", rpmOffset);

    addr = Memory::FindPattern("3B B7 ? ? ? ? 7D 0D");
    wheelsContainerOffset = addr == 0 ? 0 : *(int*)(addr + 2) - 8;
    LOG(wheelsContainerOffset == 0 ? WARN : DEBUG, "[VExt] Wheels Container offset: 0x{:03X}", wheelsContainerOffset);

    wheelCountOffset = addr == 0 ? 0 : *(int*)(addr + 2);
    LOG(wheelCountOffset == 0 ? WARN : DEBUG, "[VExt] Wheel Count offset: 0x{:03X}", wheelCountOffset);

    addr = Memory::FindPattern("45 0F 57 ? F3 0F 11 ? ? ? 00 00 F3 0F 5C");
    wheelSuspensionCompressionOffset = addr == 0 ? 0 : *(int*)(addr + 8);
    LOG(wheelSuspensionCompressionOffset == 0 ? WARN : DEBUG, "[VExt] Wheel Suspension Compression Offset: 0x%X", wheelSuspensionCompressionOffset);

    addr = Memory::FindPattern("88 8B ? ? 00 00 41 0F B6 47 51 66 89 83 ? ? 00 00");
    wheelMatTypeOffset = addr == 0 ? 0 : (*(int*)(addr + 2));
    LOG(wheelMatTypeOffset == 0 ? WARN : DEBUG, "Wheel Material Type offset: 0x{:03X}", wheelMatTypeOffset);
}

float VehicleExtensions::GetHoverTransformRatio(Vehicle handle) {
    if (hoverTransformRatioOffset == 0) return {};
    return *reinterpret_cast<float*>(Memory::GetAddressOfEntity(handle) + hoverTransformRatioOffset);
}

float VehicleExtensions::GetRPM(Vehicle handle) {
    if (rpmOffset == 0) return {};
    return *reinterpret_cast<float*>(Memory::GetAddressOfEntity(handle) + rpmOffset);
}

uint64_t VehicleExtensions::GetWheelsPtr(Vehicle handle) {
    if (wheelsContainerOffset == 0) return {};
    return *reinterpret_cast<uint64_t*>(Memory::GetAddressOfEntity(handle) + wheelsContainerOffset);
}

uint8_t VehicleExtensions::GetNumWheels(Vehicle handle) {
    if (wheelCountOffset == 0) return {};
    return *reinterpret_cast<int*>(Memory::GetAddressOfEntity(handle) + wheelCountOffset);
}

std::vector<float> VehicleExtensions::GetSuspensionCompressions(Vehicle handle) {
    if (wheelSuspensionCompressionOffset == 0) return {};
    auto wheelPtr = GetWheelsPtr(handle);
    if (wheelPtr == 0) return {};

    auto numWheels = GetNumWheels(handle);
    std::vector<float> compressions(numWheels);
    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        compressions[i] = *reinterpret_cast<float*>(wheelAddr + wheelSuspensionCompressionOffset);
    }
    return compressions;
}

std::vector<uint16_t> VehicleExtensions::GetTireContactMaterials(Vehicle handle) {
    if (wheelMatTypeOffset == 0) return {};
    auto wheelPtr = GetWheelsPtr(handle);
    if (wheelPtr == 0) return {};

    auto numWheels = GetNumWheels(handle);
    std::vector<uint16_t> values(numWheels);

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<uint16_t*>(wheelAddr + wheelMatTypeOffset));
    }
    return values;
}
