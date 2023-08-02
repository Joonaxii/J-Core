#pragma once
#include <cstdint>
#include <J-Core/Math/Vertex.h>
#include <memory>
#include <string>

namespace JCore {
    enum : uint8_t {
        Spr_None = 0,

        Spr_FromAtlas = 0x1,
        Spr_Rotated = 0x2, //Not supported atm, probably in the future
    };

    struct SpriteRect {
        uint16_t x{0}, y{0};
        uint16_t width{0}, height{0};

        SpriteRect() : 
            x(0), y(0), width(0), height(0) {}
        SpriteRect(int32_t x, int32_t y, int32_t width, int32_t height) : 
            x(x), y(y), width(width), height(height) {}
    };

    struct SpriteInfo {
        std::string name{""};
        uint8_t flags{0};
        SpriteRect rect{};

        SpriteInfo() : name(""), rect(), flags(0) {}
        SpriteInfo(const std::string& name, SpriteRect rect) : name(name), rect(rect), flags(0) {}
        SpriteInfo(const std::string& name, SpriteRect rect, uint8_t flags) : name(name), rect(rect), flags(flags) {}
    };

    class Texture;
    class Atlas;
    class Sprite {
    public:

        Sprite();
        Sprite(SpriteRect rect, const std::shared_ptr<Texture>& texture);
        Sprite(const std::string& name, SpriteRect rect, const std::shared_ptr<Texture>& texture);
        Sprite(const SpriteInfo& info, const std::shared_ptr<Texture>& texture);


        void setTexture(std::weak_ptr<Texture> texture);
        std::shared_ptr<Texture> getTexture() const { return _texture.lock(); }

        const Atlas* getAtlas() const { return _atlas; }

        bool isFromAtlas() const { return bool(_info.flags & Spr_FromAtlas); }
        bool isRotated() const { return bool(_info.flags & Spr_Rotated); }

        const Vertex* getVertices() const { return _verts; }
        void assignAtlas(Atlas* atlas, bool rotated);

    private:
        SpriteInfo _info;
        Atlas* _atlas;
        std::weak_ptr<Texture> _texture;

        Vertex _verts[4];
        void initVerts();
    };
}