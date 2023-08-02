#pragma once
#include <cstdint>

namespace JCore {
    template<typename T>
    struct IComparable {
        int32_t compareTo(const T& other) const {
            const T& self = *static_cast<const T*>(this);
            return self < other ? -1 : self > other ? 1 : 0;
        }
    };
}