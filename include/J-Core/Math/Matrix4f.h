#pragma once
#include <cstdint>
#include <array>
#include <J-Core/Math/Rect.h>
#include <glm.hpp>

namespace JCore {
    struct Matrix4f
    {
        static const Matrix4f Zero;
        static const Matrix4f Identity;

        Matrix4f();
        Matrix4f(const float a00, const float a01, const float a02,
            const float a10, const float a11, const float a12,
            const float a20, const float a21, const float a22);
        Matrix4f(const std::array<float, 16>& arr);
        Matrix4f(const float* input);
 
        Matrix4f(const glm::vec2& position, const float rotation, const glm::vec2& scale);

        Matrix4f& setTRS(const glm::vec2& position, const float rotation, const glm::vec2& scale);

        float& operator[](const int32_t i);
        const float& operator[](const int32_t i) const;

        const float* getMatrix() const;

        Matrix4f getInverse() const;

        float& at(const int32_t column, const int32_t row);
        const float& at(const int32_t column, const int32_t row) const;

        Matrix4f& combine(const Matrix4f& matrix);

        glm::vec2 transformPoint(const float x, const float y) const;
        glm::vec2 transformPoint(const glm::vec2& vec) const;

        Rect<float> transformRect(const glm::vec2& min, const glm::vec2& max) const;
        Rect<float> transformRect(const Rect<float>& rectangle) const;

        void decompose(glm::vec2& position, float& rotation, glm::vec2& scale) const;

        Rect<float>& transformRect(Rect<float>& rectangle) const;

        Matrix4f& translate(const float x, const float y);
        Matrix4f& translate(const glm::vec2& offset);

        Matrix4f& rotate(const float angle);
        Matrix4f& rotate(const float angle, const float centerX, const float centerY);
        Matrix4f& rotate(const float angle, const glm::vec2& offset);

        Matrix4f& scale(const float scaleX, const float scaleY);
        Matrix4f& scale(const glm::vec2& scale);

        Matrix4f& scale(const float scaleX, const float scaleY, const float centerX, const float centerY);
        Matrix4f& scale(const float scaleX, const float scaleY, const glm::vec2& center);

        Matrix4f& scale(const glm::vec2& scale, const float centerX, const float centerY);
        Matrix4f& scale(const glm::vec2& scale, const glm::vec2& center);

        static Matrix4f ortho(const float left, const float right, const float bottom, const float top);
        static Matrix4f ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);

    private:
        float _mat[16];
    };

    const bool operator ==(const Matrix4f& lhs, const Matrix4f& rhs);
    const bool operator !=(const Matrix4f& lhs, const Matrix4f& rhs);

    const glm::vec2 operator *(const Matrix4f& lhs, const glm::vec2& rhs);

    const Matrix4f operator *(const Matrix4f& lhs, const Matrix4f& rhs);
    Matrix4f& operator *=(Matrix4f& lhs, const Matrix4f& rhs);
}