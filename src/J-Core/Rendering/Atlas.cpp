#include <J-Core/Rendering/Atlas.h>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Log.h>

namespace JCore {
    Atlas::Atlas() : _texture(), _sprites{}, _nameToIndex{} { }

    Atlas::~Atlas() {
        if (_texture) {
            _texture->release();
            _texture.reset();
        }
    }

    void Atlas::setSprites(const SpriteInfo* frames, size_t frameCount) {
        _sprites.clear();
        _sprites.reserve(frameCount);

        for (size_t i = 0; i < frameCount; i++)  {
            _sprites.emplace_back(frames[i], _texture).assignAtlas(this, frames[i].flags & Spr_Rotated);
        }
    }

    void Atlas::applyPixelData(const ImageData& imageData) {
        if (!_texture) {
            _texture = std::make_shared<Texture>();
            for (auto& sprite : _sprites) {
                sprite.setTexture(_texture);
            }
        }

        if (_texture){
            if (_texture->getWidth() != imageData.width || _texture->getHeight() != imageData.height) {
                JCORE_ERROR("[J-Core - Atlas] Error: Failed to apply pixel data to atlas! (Invalid resolution '{0}x{1}' should be '{2}x{3}')", 
                    _texture->getWidth(), _texture->getHeight(), imageData.width, imageData.height);
                return;
            }
            if (_texture->create(imageData.data, imageData.format, imageData.paletteSize, imageData.width, imageData.height, imageData.flags)) { return; }
        }
        JCORE_ERROR("[J-Core - Atlas] Error: Failed to apply pixel data to atlas!");
    }

    const Sprite* Atlas::findByName(const std::string& str) const {
        auto find = _nameToIndex.find(str);
        if (find != _nameToIndex.end()) {
            return &_sprites[find->second];
        }
        return nullptr;
    }

    bool Atlas::isValid() const {
        return _texture && _texture->isValid();
    }
}