#include <J-Core/Rendering/Renderable.h>
#include <J-Core/Rendering/Renderer.h>

namespace JCore {
    Renderable::Renderable() : _layer(), _color(Color32::White) {
        Renderer::addRenderable(this);
    } 
    
    Renderable::~Renderable() { Renderer::removeRenderable(this); }
}