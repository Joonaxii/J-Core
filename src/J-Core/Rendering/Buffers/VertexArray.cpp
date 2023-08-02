#include <J-Core/Rendering/Buffers/VertexArray.h>
#include <GL/glew.h>
#include <J-Core/Rendering/Buffers/VertexBuffer.h>
#include <J-Core/Rendering/Buffers/BufferLayout.h>

namespace JCore {
    VertexArray::VertexArray() : _rendererID(0) { }
    VertexArray::~VertexArray() {
        release();
    }

    void VertexArray::init() {
        if (!_rendererID) {
            glGenVertexArrays(1, &_rendererID);
        }
    }

    void VertexArray::release() {
        if (_rendererID) {
            glDeleteVertexArrays(1, &_rendererID);
            _rendererID = 0;
        }
    }

    void VertexArray::addBuffer(const VertexBuffer& vb, const BufferLayout& layout) {
        if (vb.bind() && _rendererID) {
            bind();

            const auto& elements = layout.getElements();
            uint64_t offset = 0;
            for (uint32_t i = 0; i < elements.size(); i++) {
                const auto& element = elements[i];
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)(offset));

                offset += uint64_t(element.count) * BufferElement::getSizeOfType(element.type);
            }
        }
    }

    void VertexArray::bind() const {
        glBindVertexArray(_rendererID);
    }
    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }
}