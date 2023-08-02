#include <GL/glew.h>
#include <J-Core/Rendering/Shader.h>
#include <malloc.h>
#include <iostream>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Log.h>

namespace JCore {
    Shader::Shader() : _shaderId(0) { }
    Shader::~Shader() { release(); }

    bool Shader::createShader(const char* vert, const char* frag)  {
        release();

        uint32_t program = glCreateProgram();
        uint32_t vs = compileShader(vert, GL_VERTEX_SHADER);
        uint32_t fs = compileShader(frag, GL_FRAGMENT_SHADER);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        _shaderId = program;
        return _shaderId != 0;
    }

    void Shader::release() {
        if (_shaderId) {
            glDeleteProgram(_shaderId);
            _shaderId = 0;
        }
    }

    bool Shader::bind() const {
        if (_shaderId) {
            glUseProgram(_shaderId);
            return true;
        }
        return false;
    }
    void Shader::unbind() const { glUseProgram(0); }

    void Shader::setUniformMat4f(const std::string& name, const Matrix4f& mat) {
        if (!_shaderId) { return; }
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0]);
    }

    uint32_t Shader::setTextures(const std::string& name, const Texture* texture, const uint32_t position) {
        if (!texture || !_shaderId) { return position; }

        uint32_t pos = position;
        int32_t uId = getUniformLocation(name);

        if (uId < 0) { return pos; }
        glUniform1i(uId, pos++);
        if (texture->getFormat() == TextureFormat::Indexed8) {
            uId = getUniformLocation(name + "_Pal");
            if (uId > -1) {
                glUniform1i(uId, pos++);
            }
        }
        return pos;
    }

    uint32_t Shader::setTexture(const std::string& name, const uint32_t position) {
        if (!_shaderId) { return position; }

        uint32_t pos = position;
        int32_t uId = getUniformLocation(name);

        if (uId < 0) { return pos; }
        glUniform1i(uId, pos++);

        return pos;
    }

    int32_t Shader::getUniformLocation(const std::string& name) {
        if (!_shaderId) { return -1; }
        if (_uniformCache.find(name) != _uniformCache.end()) { return _uniformCache[name]; }

        int32_t location = glGetUniformLocation(_shaderId, name.c_str());
        if (location == -1) {
            JCORE_WARN("Warning: Shader uniform '{0}' doesn't exist!", name);
        }
        _uniformCache[name] = location;
        return location;
    }

    uint32_t Shader::compileShader(const char* shader, uint32_t type) {
        uint32_t shaderId = glCreateShader(type);
        glShaderSource(shaderId, 1, &shader, NULL);
        glCompileShader(shaderId);

        int result;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int len;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len);

            char* message = (char*)_malloca(len * sizeof(char));
            glGetShaderInfoLog(shaderId, len, &len, message);

            JCORE_ERROR("Error: Failed to compile {0} shader!\n{1}\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), message);
            glDeleteShader(shaderId);
            _freea(message);
            return 0;
        }
        return shaderId;
    }
}