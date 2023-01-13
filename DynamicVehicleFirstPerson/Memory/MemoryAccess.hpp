#pragma once

#include <cstdint>

namespace Memory {
    void Init();
    uintptr_t FindPattern(const char* pattern, const char* mask);
    uintptr_t FindPattern(const char* pattStr);

    extern uintptr_t(*GetAddressOfEntity)(int entity);
    extern uintptr_t(*GetModelInfo)(unsigned int modelHash, int* index);

    float GetTimeScale();
}
