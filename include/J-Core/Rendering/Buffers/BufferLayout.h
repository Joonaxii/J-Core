#pragma once
#include <vector>
#include <GL/glew.h>
#include <J-Core/Log.h>

namespace JCore {
    struct BufferElement {
        uint32_t type;
        uint32_t count;
        uint8_t normalized;

        static uint32_t getSizeOfType(const uint32_t type) {
            switch (type)
            {
                case GL_FLOAT:          return 4;
                case GL_UNSIGNED_INT:   return 4;
                case GL_UNSIGNED_BYTE:  return 1;
            }
            return 0;
        }
    };

    class BufferLayout {
    public:

        BufferLayout() : _stride(0) {  }

        template<typename T>
        void push(const uint32_t count) {
            JCORE_ERROR("Buffer Layout has no implementation for type '{0}'", typeid(T).name());
        }

        template<>
        void push<float>(const uint32_t count) {
            _elements.push_back({ GL_FLOAT, count, GL_FALSE });
            _stride += BufferElement::getSizeOfType(GL_FLOAT) * count;
        }

        template<>
        void push<uint32_t>(const uint32_t count) {
            _elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
            _stride += BufferElement::getSizeOfType(GL_UNSIGNED_INT) * count;
        }

        template<>
        void push<uint8_t>(const uint32_t count) {
            _elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
            _stride += BufferElement::getSizeOfType(GL_UNSIGNED_BYTE) * count;
        }

        inline const std::vector<BufferElement>& getElements() const { return _elements; }
        inline const uint32_t getStride() const { return _stride; }

    private:
        std::vector<BufferElement> _elements;
        uint32_t _stride;
    };
}