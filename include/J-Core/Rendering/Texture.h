#pragma once
#include <cstdint>
#include <J-Core/Math/Color32.h>
#include <unordered_map>
#include <J-Core/IO/ImageUtils.h>
#include <memory>

namespace JCore {

    class Texture {
    public:
        Texture();
        Texture(Texture&& other) noexcept;
        ~Texture() noexcept;

        TextureFormat getFormat() const { return _format; }

        bool create(const uint8_t* input, TextureFormat format, int32_t paletteSize, int32_t width, int32_t height, uint8_t flags);

        bool isValid() const { return bool(_textureId) && _valid; }

        int32_t getWidth() const { return _width; }
        int32_t getHeight() const { return _height; }

        uint32_t getTextureId() const { return _textureId; }
        uint32_t getPaletteId() const { return _paletteId; }

        int32_t getPaletteSize() const { return _paletteSize; }

        uint8_t* getPixels() const;
        uint8_t* getPixels(uint8_t* buffer) const;

        uint32_t bind(uint32_t slot) const;
        uint32_t unBind(uint32_t slot) const;

        static uint32_t bind(TextureFormat format, uint32_t slot, uint32_t id0, uint32_t id1 = 0, int8_t mipLevel = -1, int32_t override02D = -1, int32_t override12D = -1);
        static uint32_t unbind(TextureFormat format, uint32_t slot, bool resetMip = false, int32_t override02D = -1, int32_t override12D = -1);

        uint32_t getHash() const { return _crcTex; }
        uint8_t getFlags() const { return _flags; }

        void invalidate() {
            _valid = false;
        }
        void release();
    private:
        uint32_t _textureId;
        uint32_t _paletteId;

        uint32_t _crcTex;

        int32_t _paletteSize;
        uint16_t _width;
        uint16_t _height;
        TextureFormat _format;
        uint8_t _flags{ 0 };
        bool _valid{ false };

        void releaseTexture();
        void releasePalette();
    };

    enum : uint8_t {
        TEX_GEN_FREE,
        TEX_GEN_IDLE,
        TEX_GEN_WAIT,
        TEX_GEN_PROCESSING,
    };

    static constexpr size_t MAX_TEXTURES_QUEUED{ 16 };
    struct TextureGenState {
        std::shared_ptr<Texture>* texture{};
        ImageData data{};
        uint8_t state{ TEX_GEN_FREE };
    };

    void waitForTexGen(int32_t index, size_t sleepFor = 100);
    bool shouldWaitForTexGen(int32_t index);

    template<size_t count>
    void waitForTexGen(int32_t(&indices)[count], size_t sleepFor = 100) {
        while (true) {
            for (size_t i = 0; i < count; i++) {
                if (shouldWaitForTexGen(indices[i])) {
                    goto keepGoing;
                }
            }
            break;
            keepGoing:
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
        }
    }

    int32_t setupTexGen(std::shared_ptr<Texture>& texture, const ImageData& data);
    void updateTexGen();

}