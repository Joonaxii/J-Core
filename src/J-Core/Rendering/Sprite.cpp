#include <J-Core/Rendering/Sprite.h>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Rendering/Atlas.h>

namespace JCore {
    static glm::vec2 pixToUVCoord(const glm::i32vec2& coord, const glm::i32vec2& size) {
        return { size.x ? coord.x / float(size.x) : 0, size.y ? coord.y / float(size.y) : 0 };
    }

    Sprite::Sprite() : _texture(), _info(), _atlas(nullptr) { }
    Sprite::Sprite(SpriteRect rect, const std::shared_ptr<Texture>& texture)
        : _texture(texture), _info("", rect),  _atlas(nullptr) {
        initVerts();
    }

    Sprite::Sprite(const std::string& name, SpriteRect rect, const std::shared_ptr<Texture>& texture) 
        : _texture(texture), _info(name, rect), _atlas(nullptr) {
        initVerts();
    }

    Sprite::Sprite(const SpriteInfo& info, const std::shared_ptr<Texture>& texture)
        : _texture(texture), _info(info), _atlas(nullptr) {
        initVerts();
    }

    void Sprite::setTexture(std::weak_ptr<Texture> texture) {
        _texture = texture;
    }

    void Sprite::assignAtlas(Atlas* atlas, bool rotated) {
        _atlas = atlas;
        _info.flags = (rotated ? Spr_Rotated : 0) | (atlas ? Spr_FromAtlas : 0);
    }

    void Sprite::initVerts()  {
        const auto& rect = _info.rect;
        auto texPtr = _texture.lock();
        glm::i32vec2 reso = texPtr ? glm::i32vec2(texPtr->getWidth(), texPtr->getHeight()) : glm::i32vec2(0, 0);
        _verts[0] = { {rect.width * -0.5f, rect.height * -0.5f}, {0xFF, 0xFF, 0xFF, 0xFF}, pixToUVCoord({rect.x, rect.y}, reso) };
        _verts[1] = { {rect.width * 0.5f, rect.height * -0.5f}, {0xFF, 0xFF, 0xFF, 0xFF}, pixToUVCoord({rect.x + rect.width, rect.y}, reso) };
        _verts[2] = { {rect.width * 0.5f, rect.height * 0.5f}, {0xFF, 0xFF, 0xFF, 0xFF}, pixToUVCoord({rect.x + rect.width, rect.y + rect.height}, reso) };
        _verts[3] = { {rect.width * -0.5f, rect.height * 0.5f}, {0xFF, 0xFF, 0xFF, 0xFF}, pixToUVCoord({rect.x,              rect.y + rect.height}, reso) };
    }
}