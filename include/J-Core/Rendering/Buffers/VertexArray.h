#pragma once
#include <cstdint>

namespace JCore {
    class VertexBuffer;
    class BufferLayout;
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void init();
        void release();

        void addBuffer(const VertexBuffer& vb, const BufferLayout& layout);

        void bind() const;
        void unbind() const;

    private:
        uint32_t _rendererID;
    };
}