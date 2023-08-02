#include <J-Core/Rendering/Buffers/FrameBuffer.h>
#include <GL/glew.h>
#include <cassert>
#include <J-Core/Rendering/Renderer.h>

namespace JCore {

    uint32_t getInverseGLFormat(uint32_t gl) {
        switch (gl)
        {
            default: return gl;
            case GL_RGB8: return GL_RGB;
            case GL_RGBA8: return GL_RGBA;
            case GL_R8: return GL_RED;
            case GL_RG8: return GL_RG;
        }
    }

    FrameBuffer::FrameBuffer() : FrameBuffer(GL_RGBA8) { }
    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept : 
        _bufferId(std::exchange(other._bufferId, 0)),
        _colorAttatchment(std::exchange(other._colorAttatchment, 0)),
        _specs(std::exchange(other._specs, {})), _projMatrix(other._projMatrix), 
        _dirty(other._dirty)
    {

    }
    FrameBuffer::FrameBuffer(const uint32_t colorFormat) : 
        _specs(), _bufferId(0), _colorAttatchment(0), _projMatrix(Matrix4f::Identity), _dirty(false) { }

    FrameBuffer::FrameBuffer(const FrameBufferSpecs& specs) : FrameBuffer(GL_RGBA8, specs) { }
    FrameBuffer::FrameBuffer(const uint32_t colorFormat, const FrameBufferSpecs& specs) : 
        _specs(specs), _bufferId(0), _colorAttatchment(0), _projMatrix(Matrix4f::Identity), _dirty(false) { }

    FrameBuffer::~FrameBuffer() noexcept {
        releaseBuffer();
        releaseColorAttatchment();
    }

    void FrameBuffer::refresh() const {
        if (_dirty) {
            _dirty = false;
            invalidate(true);
        }
    }

    void FrameBuffer::invalidate() const {
        const bool rebuild = !_bufferId;
        invalidate(rebuild);
    }

    void FrameBuffer::invalidate(const FrameBufferSpecs& specs, const bool instant, const bool force) const {
        const bool rebuild = specs != _specs || !_bufferId || force;
        _specs = specs;

        if (instant) {
            invalidate(rebuild);
            return;
        }
        _dirty |= rebuild;
    }

    void FrameBuffer::invalidate(const bool rebuild) const {
        if (!rebuild) { return; }

        _projMatrix = Matrix4f::ortho(_specs.width * -0.5f, _specs.width * 0.5f, _specs.height * -0.5f, _specs.height * 0.5f);

        const bool createdBuf = !_bufferId;
        const bool createdTex = !_colorAttatchment;

        if (createdBuf) {
            glCreateFramebuffers(1, &_bufferId);
        }

        if (createdTex || createdBuf) {
            glBindFramebuffer(GL_FRAMEBUFFER, _bufferId);
        }

        if (createdTex) {
            glCreateTextures(GL_TEXTURE_2D, 1, &_colorAttatchment);
        }

        glBindTexture(GL_TEXTURE_2D, _colorAttatchment);
        glTexImage2D(GL_TEXTURE_2D, 0, _specs.colorFormat, _specs.width, _specs.height, 0, getInverseGLFormat(_specs.colorFormat), GL_UNSIGNED_BYTE, _specs.pixelData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (createdTex || createdBuf) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorAttatchment, 0);
        }

        if (createdTex ) {
            GLenum ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(ret == GL_FRAMEBUFFER_COMPLETE && "Framebuffer not complete!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::releaseBuffer() const noexcept {
        if (_bufferId) {
            glDeleteFramebuffers(1, &_bufferId);
            _bufferId = 0;
        }
    }

    void FrameBuffer::releaseColorAttatchment() const noexcept {
        if (_colorAttatchment) {
           glDeleteFramebuffers(1, &_colorAttatchment);
           _colorAttatchment = 0;
        }
    }

    uint32_t FrameBuffer::bindColorAttachment(const uint32_t slot) const {
        uint32_t bindP = slot;

        glActiveTexture(GL_TEXTURE0 + bindP++);
        glBindTexture(GL_TEXTURE_2D, _colorAttatchment);
        return slot;
    }

    uint32_t FrameBuffer::unbindColorAttachment(const uint32_t slot) const {
        uint32_t bindP = slot;
        glActiveTexture(GL_TEXTURE0 + bindP++);
        glBindTexture(GL_TEXTURE_2D, 0);
        return bindP;
    }

    void FrameBuffer::blit(const uint8_t* data) {
        if (bind()) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, getGLPixelAlignment(_specs.colorFormat));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _specs.width, _specs.height, getInverseGLFormat(_specs.colorFormat), GL_UNSIGNED_BYTE, data);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
    }

    void FrameBuffer::blit(const uint8_t* data, const int32_t x, const int32_t y, const int32_t width, const int32_t height, const uint32_t format) {
        if (bind()) {

            glPixelStorei(GL_UNPACK_ALIGNMENT, getGLPixelAlignment(_specs.colorFormat));
            glPixelStorei(GL_PACK_ALIGNMENT  , getGLPixelAlignment(format));

            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, GL_UNSIGNED_BYTE, data);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glPixelStorei(GL_PACK_ALIGNMENT  , 4);
        }
    }

    bool FrameBuffer::bind() const {
        if (_bufferId) {
            glBindFramebuffer(GL_FRAMEBUFFER, _bufferId);
            return true;
        }
        return false;
    }

    void FrameBuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}