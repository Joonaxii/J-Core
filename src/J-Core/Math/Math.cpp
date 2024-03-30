#include <J-Core/Math/Math.h>

namespace JCore::Math {
    float easeInOutQuart(float t) {
        if (t <= 0) { return 0; }
        if (t >= 1) { return 1; }

        if (t < 0.5f) {
            return powf(2.0f, 20.0f * t - 10.0f) * 0.5f;
        }
        return (2.0f - powf(2.0f, -20.0f * t + 10.0f)) * 0.5f;
    }

    float easeInOutCubic(float t) {
        if (t < 0.5f) {
            return (4 * t * t * t);
        }
        return 1.0f - powf(-2 * t + 2, 3) * 0.5f;
    }

    float easeOutCubic(float t) {
        return 1.0f - powf(1.0f - t, 3);
    }
}