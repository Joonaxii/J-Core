#pragma once 
#include <glm.hpp>

namespace JCore {
    template<typename T>
    struct Rect {
        glm::vec<2, T> min;
        glm::vec<2, T> max;

        Rect() : min(0, 0), max(0, 0) {}
        Rect(const glm::vec<2, T>& min, const glm::vec<2, T>& max) :
            min(min), max(max) {}
        Rect(const T x, const T y, const T width, const T height) : 
            min(x, y), max(x + width, y + width) {}

        const glm::vec<2, T> getSize() const {
            return max - min;
        }

        const glm::vec<2, T> getCenter() const {
            return min + (getSize() / 2);
        }
    };

    template<>
    inline const glm::vec<2, float> Rect<float>::getCenter() const {
        return min + (getSize() * 0.5f);
    }

    template<>
    inline const glm::vec<2, double> Rect<double>::getCenter() const {
        return min + (getSize() * 0.5);
    }
}