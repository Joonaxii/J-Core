#include <J-Core/Math/Matrix4f.h>
#include <J-Core/Math/Math.h>

namespace JCore {
    const Matrix4f Matrix4f::Zero = Matrix4f({
       0.0f, 0.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 0.0f });

    const Matrix4f Matrix4f::Identity = Matrix4f({
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 });

    Matrix4f::Matrix4f() : _mat{ 0.0f } {
        _mat[0] = 1.0f;
        _mat[5] = 1.0f;
        _mat[10] = 1.0f;
        _mat[15] = 1.0f;
    }

    Matrix4f::Matrix4f(const float* input) {
        memcpy(_mat, input, 16 * sizeof(float));
    }

    Matrix4f::Matrix4f(const std::array<float, 16>& input) {
        memcpy(_mat, input.data(), 16 * sizeof(float));
    }

    Matrix4f::Matrix4f(const glm::vec2& position, const float rotation, const glm::vec2& scale) : Matrix4f(Matrix4f::Identity) {
        this->rotate(rotation);
        this->scale(scale);
        this->translate(position);
    }

    Matrix4f& Matrix4f::setTRS(const glm::vec2& position, const float rotation, const glm::vec2& scale) {
        *this = Matrix4f(Matrix4f::Identity);

        this->rotate(rotation);
        this->scale(scale);
        this->translate(position);

        return *this;
    }

    const float* Matrix4f::getMatrix() const {
        return _mat;
    }

    Matrix4f Matrix4f::getInverse() const {
        float det = _mat[0] * (_mat[15] * _mat[5] - _mat[7] * _mat[13]) -
            _mat[1] * (_mat[15] * _mat[4] - _mat[7] * _mat[12]) +
            _mat[3] * (_mat[13] * _mat[4] - _mat[5] * _mat[12]);

        if (det != 0.0f) {
            const float mult = 1.0f / det;

            return Matrix4f(
                (_mat[15] * _mat[5] - _mat[7] * _mat[13]) * mult,
                -(_mat[15] * _mat[4] - _mat[7] * _mat[12]) * mult,
                (_mat[13] * _mat[4] - _mat[5] * _mat[12]) * mult,
                -(_mat[15] * _mat[1] - _mat[3] * _mat[13]) * mult,
                (_mat[15] * _mat[0] - _mat[3] * _mat[12]) * mult,
                -(_mat[13] * _mat[0] - _mat[1] * _mat[12]) * mult,
                (_mat[7] * _mat[1] - _mat[3] * _mat[5]) * mult,
                -(_mat[7] * _mat[0] - _mat[3] * _mat[4]) * mult,
                (_mat[5] * _mat[0] - _mat[1] * _mat[4]) * mult);
        }
        return Identity;

    }

    Matrix4f::Matrix4f(
        const float a00, const float a01, const float a02,
        const float a10, const float a11, const float a12,
        const float a20, const float a21, const float a22) {
        _mat[0] = a00; _mat[4] = a01; _mat[8] = 0.f; _mat[12] = a02;
        _mat[1] = a10; _mat[5] = a11; _mat[9] = 0.f; _mat[13] = a12;
        _mat[2] = 0.f; _mat[6] = 0.f; _mat[10] = 1.f; _mat[14] = 0.f;
        _mat[3] = a20; _mat[7] = a21; _mat[11] = 0.f; _mat[15] = a22;
    }

    float& Matrix4f::operator[](const int32_t i) {
        assert(i > -1 && i < 16 && "Index out of range of matrix!");
        return _mat[i];
    }

    const float& Matrix4f::operator[](const int32_t i) const {
        assert(i > -1 && i < 16 && "Index out of range of matrix!");
        return _mat[i];
    }

    float& Matrix4f::at(const int32_t column, const int32_t row) {
        assert(column > -1 && column < 4 && "Column outside the range of matrix!");
        assert(row > -1 && row < 4 && "Row outside the range of matrix!");
        return (*this)[row * 4 + column];
    }

    const float& Matrix4f::at(const int32_t column, const int32_t row) const {
        assert(column > -1 && column < 4 && "Column outside the range of matrix!");
        assert(row > -1 && row < 4 && "Row outside the range of matrix!");
        return (*this)[row * 4 + column];
    }

    const bool operator==(const Matrix4f& lhs, const Matrix4f& rhs) {
        auto matLhs = lhs.getMatrix();
        auto matRhs = rhs.getMatrix();
        for (size_t i = 0; i < 16; i++)
        {
            if (matLhs[i] != matRhs[i]) { return false; }
        }
        return true;
    }

    const bool operator!=(const Matrix4f& lhs, const Matrix4f& rhs) {
        return !(lhs == rhs);
    }

    const glm::vec2 operator*(const Matrix4f& lhs, const glm::vec2& rhs) {
        return lhs.transformPoint(rhs);
    }

    const Matrix4f operator*(const Matrix4f& lhs, const Matrix4f& rhs) {
        return Matrix4f(lhs).combine(rhs);
    }

    Matrix4f& operator*=(Matrix4f& lhs, const Matrix4f& rhs) {
        return lhs.combine(rhs);
    }

    Matrix4f& Matrix4f::combine(const Matrix4f& matrix) {
        const float* a = _mat;
        const float* b = matrix._mat;

        *this = Matrix4f(
            a[0] * b[0] + a[4] * b[1] + a[12] * b[3],
            a[0] * b[4] + a[4] * b[5] + a[12] * b[7],
            a[0] * b[12] + a[4] * b[13] + a[12] * b[15],
            a[1] * b[0] + a[5] * b[1] + a[13] * b[3],
            a[1] * b[4] + a[5] * b[5] + a[13] * b[7],
            a[1] * b[12] + a[5] * b[13] + a[13] * b[15],
            a[3] * b[0] + a[7] * b[1] + a[15] * b[3],
            a[3] * b[4] + a[7] * b[5] + a[15] * b[7],
            a[3] * b[12] + a[7] * b[13] + a[15] * b[15]);
        return *this;
    }

    glm::vec2 Matrix4f::transformPoint(const float x, const float y) const {
        return glm::vec2(_mat[0] * x + _mat[4] * y + _mat[12],
            _mat[1] * x + _mat[5] * y + _mat[13]);
    }

    glm::vec2 Matrix4f::transformPoint(const glm::vec2& vec) const {
        return transformPoint(vec.x, vec.y);
    }

    Rect<float> Matrix4f::transformRect(const glm::vec2& min, const glm::vec2& max) const {
        const glm::vec2 points[4]{
            transformPoint(min),
            transformPoint(min.x, max.y),
            transformPoint(max),
            transformPoint(max.x, min.y),
        };

        glm::vec2 minO(points[0]);
        glm::vec2 maxO(points[0]);
        for (size_t i = 1; i < 4; i++) {
            const auto& vec = points[i];
            minO.x = vec.x < minO.x ? vec.x : minO.x;
            maxO.x = vec.x > maxO.x ? vec.x : maxO.x;

            minO.y = vec.y < minO.y ? vec.y : minO.y;
            maxO.y = vec.y > maxO.y ? vec.y : maxO.y;
        }
        return Rect<float>(minO, maxO);
    }

    Rect<float> Matrix4f::transformRect(const Rect<float>& rectangle) const {
        return transformRect(rectangle.min, rectangle.max);
    }

    void Matrix4f::decompose(glm::vec2& position, float& rotation, glm::vec2& scale) const {
        position.x = _mat[12];
        position.y = _mat[13];

        const float m00 = _mat[0];
        const float m01 = _mat[1];

        const float m10 = _mat[4];
        const float m11 = _mat[5];

        scale.x = Math::sign(m00) * std::sqrtf((m00 * m00) + (m01 * m01));
        scale.y = Math::sign(m11) * std::sqrtf((m10 * m10) + (m11 * m11));

        rotation = std::atan2f(m10, m11);
    }

    Rect<float>& Matrix4f::transformRect(Rect<float>& rectangle) const {
        auto& min = rectangle.min;
        auto& max = rectangle.max;
        const glm::vec2 points[4]{
             transformPoint(min),
             transformPoint(min.x, max.y),
             transformPoint(max),
             transformPoint(max.x, min.y),
        };

        glm::vec2 minO(points[0]);
        glm::vec2 maxO(points[0]);
        for (size_t i = 1; i < 4; i++) {
            const auto& vec = points[i];
            minO.x = vec.x < minO.x ? vec.x : minO.x;
            maxO.x = vec.x > maxO.x ? vec.x : maxO.x;

            minO.y = vec.y < minO.y ? vec.y : minO.y;
            maxO.y = vec.y > maxO.y ? vec.y : maxO.y;
        }
        rectangle.min = minO;
        rectangle.max = maxO;
        return rectangle;
    }

    Matrix4f& Matrix4f::translate(const float x, const float y) {
        Matrix4f translation(1, 0, x,
            0, 1, y,
            0, 0, 1);
        return combine(translation);
    }

    Matrix4f& Matrix4f::translate(const glm::vec2& offset) {
        return translate(offset.x, offset.y);
    }

    Matrix4f& Matrix4f::rotate(const float angle) {
        const float rad = angle * DEG_2_RAD;
        const float cos = std::cos(rad);
        const float sin = std::sin(rad);

        Matrix4f rotation(cos, -sin, 0,
            sin, cos, 0,
            0, 0, 1);

        return combine(rotation);
    }

    Matrix4f& Matrix4f::rotate(const float angle, const float centerX, const float centerY) {
        const float rad = angle * DEG_2_RAD;
        const float cos = std::cos(rad);
        const float sin = std::sin(rad);

        Matrix4f rotation(cos, -sin, centerX * (1 - cos) + centerY * sin,
            sin, cos, centerY * (1 - cos) - centerX * sin,
            0, 0, 1);
        return combine(rotation);
    }

    Matrix4f& Matrix4f::rotate(const float angle, const glm::vec2& offset) {
        return rotate(angle, offset.x, offset.y);
    }

    Matrix4f& Matrix4f::scale(const float scaleX, const float scaleY) {
        Matrix4f scaling(scaleX, 0, 0,
            0, scaleY, 0,
            0, 0, 1);
        return combine(scaling);
    }

    Matrix4f& Matrix4f::scale(const glm::vec2& scale) {
        return this->scale(scale.x, scale.y);
    }

    Matrix4f& Matrix4f::scale(const float scaleX, const float scaleY, const float centerX, const float centerY) {
        Matrix4f scaling(scaleX, 0, centerX * (1 - scaleX),
            0, scaleY, centerY * (1 - scaleY),
            0, 0, 1);

        return combine(scaling);
    }

    Matrix4f& Matrix4f::scale(const float scaleX, const float scaleY, const glm::vec2& center) {
        return scale(scaleX, scaleY, center.x, center.y);
    }

    Matrix4f& Matrix4f::scale(const glm::vec2& scale, const float centerX, const float centerY) {
        return this->scale(scale.x, scale.y, centerX, centerY);
    }

    Matrix4f& Matrix4f::scale(const glm::vec2& scale, const glm::vec2& center) {
        return this->scale(scale.x, scale.y, center.x, center.y);
    }

    Matrix4f Matrix4f::ortho(const float left, const float right, const float bottom, const float top) {
        Matrix4f mat = Matrix4f::Identity;

        mat.at(0, 0) = 2.0f / (right - left);
        mat.at(1, 1) = 2.0f / (top - bottom);
        mat.at(2, 2) = 1.0f;
        mat.at(3, 0) = (right + left) / (right - left);
        mat.at(3, 1) = (top + bottom) / (top - bottom);
        return mat;
    }

    Matrix4f Matrix4f::ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
        Matrix4f mat = Matrix4f::Identity;

        mat.at(0, 0) = 2.0f / (right - left);
        mat.at(1, 1) = 2.0f / (top - bottom);
        mat.at(2, 2) = -2.0f / (zFar - zNear);
        mat.at(3, 0) = (right + left) / (right - left);
        mat.at(3, 1) = (top + bottom) / (top - bottom);
        mat.at(3, 2) = zNear / (zFar - zNear);
        return mat;
    }
}