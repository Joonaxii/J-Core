#pragma once
#include <J-Core/Rendering/SortingLayer.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Vertex.h>
#include <J-Core/Math/Matrix4f.h>
#include <J-Core/Rendering/Texture.h>
#include <memory>

namespace JCore {
    class Renderable {
    public:
        Renderable();
        virtual ~Renderable();

        virtual bool canRender() const = 0;
        virtual uint32_t getVertexCount() const = 0;
        virtual const Vertex* getVertices() const = 0;
        virtual uint32_t getIndexCount() const = 0;
        virtual const uint32_t* getIndices() const = 0;
        virtual std::shared_ptr<Texture> getTexture() const = 0;

        virtual const Matrix4f& getWorldMatrix() const = 0;

        const SortingLayer& getSortingLayer() const { return _layer; }
        SortingLayer& getSortingLayer() { return _layer; }

        Color32 setColor(const Color32& color) { return _color = color; }
        Color32 getColor() const { return _color; }

    protected:
        SortingLayer _layer;
        Color32 _color;
    };
}