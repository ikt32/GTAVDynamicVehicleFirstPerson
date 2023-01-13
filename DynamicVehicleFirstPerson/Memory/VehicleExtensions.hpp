#pragma once
#include <inc/types.h>
#include <cstdint>
#include <vector>

namespace VehicleExtensions {
    void Init();

    float GetHoverTransformRatio(Vehicle handle);
    float GetRPM(Vehicle handle);
    uint64_t GetWheelsPtr(Vehicle handle);
    uint8_t GetNumWheels(Vehicle handle);
    std::vector<float> GetSuspensionCompressions(Vehicle handle);
    std::vector<uint16_t> GetTireContactMaterials(Vehicle handle);
}

// harold_thumbs_up.webp
namespace VExt = VehicleExtensions;
