#include <J-Core/Rendering/Buffers/IndexBuffer.h>
#include <GL/glew.h>

namespace JCore {
    IndexBuffer::IndexBuffer() : _rendererID(0), _count(0) {}
    IndexBuffer::IndexBuffer(const uint32_t* data, const uint32_t count, const uint32_t drawMode) : _count(count) {
        init(data, count, drawMode);
    }

    IndexBuffer::~IndexBuffer() { release(); }

    void IndexBuffer::init(const uint32_t* data, const uint32_t count, const uint32_t drawMode) {
        _count = count;
        if (!_rendererID) {
            glGenBuffers(1, &_rendererID);
            glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), data, drawMode);
        }
    }

    void IndexBuffer::release()  {
        if (_rendererID) {
            glDeleteBuffers(1, &_rendererID);
            _rendererID = 0;
        }
    }

    void IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
    }

    void IndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void IndexBuffer::updateBuffer(const uint32_t* data, const uint32_t count) {
        _count = count;
        if (_rendererID) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(uint32_t), data);
        }
    }

    uint32_t IndexBuffer::getCount() const { return _count; }
}