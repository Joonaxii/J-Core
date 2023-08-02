#pragma once
#include <J-Core/Rendering/Shader.h>
#include <J-Core/Math/Vertex.h>
#include <J-Core/Rendering/Buffers/BufferLayout.h>
#include <J-Core/Rendering/Buffers/IndexBuffer.h>
#include <J-Core/Rendering/Buffers/VertexBuffer.h>
#include <J-Core/Rendering/Buffers/VertexArray.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Matrix4f.h>

namespace JCore {
    class DynamicBatch {
    public:
        static constexpr size_t MAX_QUADS = 1 << 13;
        static constexpr size_t MAX_VERTS = MAX_QUADS << 2;
        static constexpr size_t MAX_INDICES = MAX_QUADS * 6;

        DynamicBatch() :
            _vertexBuffer(nullptr), _indexBuffer(nullptr), _shader(nullptr),
            _vertCount(0), _indCount(0),
            _ib(), _vb(), _va() {  }

        ~DynamicBatch() {
            release();
        }

        void init(const BufferLayout& bufferLayout, const uint32_t vertCount = MAX_VERTS, const uint32_t indCount = MAX_INDICES) {
            if (vertCount) {
                _vertexBuffer = reinterpret_cast<Vertex*>(malloc(vertCount * sizeof(Vertex)));
                _vb.init(nullptr, vertCount * sizeof(Vertex), GL_DYNAMIC_DRAW);

                _va.init();
                _va.addBuffer(_vb, bufferLayout);
            }

            if (indCount) {
                _indexBuffer = reinterpret_cast<uint32_t*>(malloc(indCount * sizeof(uint32_t)));
                _ib.init(nullptr, indCount, GL_DYNAMIC_DRAW);
            }
        }

        void release() {
            _va.release();
            _ib.release();
            _vb.release();

            if (_vertexBuffer) {
                free(_vertexBuffer);
                _vertexBuffer = nullptr;
            }

            if (_indexBuffer) {
                free(_indexBuffer);
                _indexBuffer = nullptr;
            }
        }

        void setup(Shader* shader) {
            _vertCount = 0;
            _indCount = 0;
            _shader = shader;
        }

        void addVerts(const Matrix4f& matrix, const Color32& color, const Vertex* verts, const uint32_t vertCount, const uint32_t* indices, const uint32_t indexCount) {
            const uint32_t indSt = _vertCount;
            for (size_t i = 0; i < vertCount; i++) {
                auto& vert = verts[i];
                _vertexBuffer[_vertCount++] = { matrix.transformPoint(vert.position), color, vert.uv };
            }

            for (size_t i = 0; i < indexCount; i++) {
                _indexBuffer[_indCount++] = indices[i] + indSt;
            }
        }

        const uint32_t getVertexCount()   const { return _vertCount; }
        const uint32_t getIndexCount()    const { return _indCount; }

        const bool drawBatch() {
            _vb.updateBuffer(_vertexBuffer, _vertCount * sizeof(Vertex));
            _vb.bind();
            _va.bind();

            _ib.updateBuffer(_indexBuffer, _indCount);
            _ib.bind();

            auto indCou = _ib.getCount();
            glDrawElements(GL_TRIANGLES, indCou, GL_UNSIGNED_INT, nullptr);
            return true;
        }
    private:
        Shader* _shader;

        uint32_t _vertCount;
        Vertex* _vertexBuffer;

        uint32_t _indCount;
        uint32_t* _indexBuffer;

        IndexBuffer _ib;
        VertexBuffer _vb;
        VertexArray _va;
    };
}