#include "VehicleExtensions.hpp"

#include "MemoryAccess.hpp"
#include "../Util/Logger.hpp"

namespace {
    int hoverTransformRatioOffset = 0;
}

void VehicleExtensions::Init() {
    // alloc8or
    auto addr = Memory::FindPattern("F3 0F 11 B3 ? ? ? ? 44 48 ? ? ? ? ? 48 85 C9");
    hoverTransformRatioOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(hoverTransformRatioOffset == 0 ? WARN : DEBUG, "[VExt] Hover Transform Active Offset: 0x%X", hoverTransformRatioOffset);
}

float VehicleExtensions::GetHoverTransformRatio(Vehicle handle) {
    if (hoverTransformRatioOffset == 0) return {};
    return *reinterpret_cast<float*>(Memory::GetAddressOfEntity(handle) + hoverTransformRatioOffset);
}
