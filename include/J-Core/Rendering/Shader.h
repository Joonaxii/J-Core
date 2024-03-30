#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <J-Core/Math/Matrix4f.h>

namespace JCore {
    class Texture;
    class Shader {
    public:
        Shader();
        ~Shader();

        bool createShader(const char* vert, const char* frag);
        void release();

        void setUniformMat4f(const std::string& name, const Matrix4f& mat);

        uint32_t setTexture(const std::string& name, const uint32_t position);
        uint32_t setTextures(const std::string& name, const Texture* texture, const uint32_t position);

        bool bind() const;
        void unbind() const;

        uint32_t getShaderId() const { return _shaderId; }

    private:
        std::unordered_map<std::string, int32_t> _uniformCache;

        uint32_t _shaderId;

        static uint32_t compileShader(const char* shader, uint32_t type);
        int32_t getUniformLocation(const std::string& name);
    };
}