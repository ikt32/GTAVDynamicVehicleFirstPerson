#pragma once

namespace Compatibility {
    void Setup();
    void Cleanup();
}

namespace Dismemberment {
    bool Available();

    void AddBoneDraw(int handle, int start, int end);
    void RemoveBoneDraw(int handle);
}
