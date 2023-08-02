#pragma once
#include <cstdint>

namespace JCore {
    class VertexBuffer {
    public:
        VertexBuffer();
        VertexBuffer(const void* data, const size_t size, const uint32_t drawMode);
        ~VertexBuffer();

        void init(const void* data, const size_t size, const uint32_t drawMode);
        void release();
        void updateBuffer(const void* data, const size_t size);

        bool bind() const;
        void unbind() const;

    private:
        uint32_t _rendererID;
    };
}