#pragma once
#include <glm.hpp>
#include <J-Core/Math/Color32.h>

namespace JCore {
    struct Vertex {
        glm::vec2 position;
        Color32 color;
        glm::vec2 uv;
    };
}