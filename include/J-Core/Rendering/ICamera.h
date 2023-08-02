#pragma once
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Rect.h>
#include <J-Core/Math/Matrix4f.h>
#include <J-Core/Rendering/Buffers/FrameBuffer.h>

namespace JCore {
    class Object;
    class ICamera {
    public:
        ICamera() : _clearColor(Color32::Clear), _ownerObject{nullptr}, _viewRect(0, 0, 1, 1), _screenRect(0, 0, 1, 1) {}
        ICamera(Object* obj) : _clearColor(Color32::Clear), _ownerObject{obj}, _viewRect(0, 0, 1, 1), _screenRect(0, 0, 1, 1) {}

        Object* getOwningObject() { return _ownerObject; }

        virtual const Matrix4f& getViewMatrix(Matrix4f& view) const = 0;
        virtual const Matrix4f& getWorldMatrix() const = 0;

        const Rect<float>& getViewRect() const { return _viewRect; }
        Rect<float>& getViewRect() { return _viewRect; }

        const Rect<float>& getScreenRect() const { return _screenRect; }
        Rect<float>& getScreenRect() { return _screenRect; }

        Color32 setClearColor(const Color32 color) { return _clearColor = color; }
        Color32 getClearColor() const { return _clearColor; }

        virtual const FrameBuffer* getFrameBuffer() const = 0;

    protected:
        Object* _ownerObject;

        Color32 _clearColor;
        Rect<float> _viewRect;
        Rect<float> _screenRect;
    };
}