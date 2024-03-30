#include <GL/glew.h>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Rendering/Renderer.h>
#include <J-Core/Math/Math.h>
#include <J-Core/Util/DataUtils.h>

namespace JCore {
    Texture::Texture() : _width(0), _height(0), _textureId(0), _paletteId(0), _paletteSize(0), _crcTex(0), _format(TextureFormat::Unknown) { }
    Texture::Texture(Texture&& other) noexcept : 
        _width(other._width), _height(other._height), 
        _paletteSize(other._paletteSize),
        _format(other._format),
        _crcTex(other._crcTex),
        _textureId(std::exchange(other._textureId, 0)),
        _paletteId(std::exchange(other._paletteId, 0))
         {
    }
    Texture::~Texture() noexcept {
        release();
    }

    bool Texture::create(const uint8_t* input, TextureFormat format, int32_t paletteSize, int32_t width, int32_t height, uint8_t flags) {
        _valid = false;
        const bool isIndexed = format == TextureFormat::Indexed8 || format == TextureFormat::Indexed16;
        if (isIndexed && !Math::isAlignedToPalette(paletteSize)) {
            JCORE_WARN("Couldn't create texture, palette is not aligned to palette scan! ({0}, should be {1})", paletteSize, Math::alignToPalette(paletteSize));
            return false;
        }

        if (format == TextureFormat::Unknown || width == 0 || height == 0) {
            JCORE_WARN("Couldn't create texture, given data was invalid! ({0}, {1}x{2})", getTextureFormatName(format), width, height);
            return false;
        }

        _flags = flags;
        if (format == TextureFormat::Indexed8) { paletteSize = 256; }
        size_t extra = isIndexed ? paletteSize * 4 : 0;
        const uint8_t* pixels = input + extra;

        _crcTex = 0xFFFFFFFFU;
        _crcTex = Data::updateCRC(_crcTex, input, std::min<size_t>((width * height * (getBitsPerPixel(format) >> 3)) + extra, 1024)) ^ 0xFFFFFFFFU;

        _paletteSize = paletteSize;
        if (!_textureId) {
            glGenTextures(1, &_textureId);
        }

        _format = format;
        _width = uint16_t(width);
        _height = uint16_t(height);


        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, getGLPixelAlignment(format));
        glTexImage2D(GL_TEXTURE_2D, 0, textureFormatToGLFormat(format, true), width, height, 0, textureFormatToGLFormat(format, false), textureFormatToGLType(_format), input ? pixels : nullptr);

        if (!input) {
            return true;
        }

        if (isIndexed) {
            if (!_paletteId) {
                glGenTextures(1, &_paletteId);
            }

            uint32_t texType = format == TextureFormat::Indexed16 ? GL_TEXTURE_2D : GL_TEXTURE_1D;

            glBindTexture(texType, _paletteId);
            glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            if (format == TextureFormat::Indexed16) {
                uint32_t palSize = paletteSize >> 8;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, palSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, input);
            }
            else {
                glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, paletteSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, input);
            }

        }
        else {
            releasePalette();
        }

        _valid = true;
        return true;
    }


    uint32_t Texture::bind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot++);
        glBindTexture(GL_TEXTURE_2D, _textureId);

        if (_format == TextureFormat::Indexed8) {
            glActiveTexture(GL_TEXTURE0 + slot++);
            glBindTexture(GL_TEXTURE_1D, _paletteId);
        }
        return slot;
    }

    uint32_t Texture::unBind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot++);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (_format == TextureFormat::Indexed8) {
            glActiveTexture(GL_TEXTURE0 + slot++);
            glBindTexture(GL_TEXTURE_1D, 0);;
        }
        return slot;
    }

    uint8_t* Texture::getPixels() const {
        size_t size = _width * _height * getBitsPerPixel(_format);
        if (_format == TextureFormat::Indexed8) {
            size +=  1024;
        }
        else if (_format == TextureFormat::Indexed16) {
            size += _paletteSize * 4;
        }
        uint8_t* pixels = reinterpret_cast<uint8_t*>(malloc(size));
        return getPixels(pixels);
    }

    uint8_t* Texture::getPixels(uint8_t* buffer) const {
        if (!_textureId || !buffer) { return nullptr; }

        size_t bpp = getBitsPerPixel(_format);
        size_t offset = 0;
        size_t size = _width * _height * bpp;
        if (_format == TextureFormat::Indexed8) {
            size += offset = 1024;
        }
        else if(_format == TextureFormat::Indexed16) {
            size += offset = _paletteSize * 4;
        }

        glBindTexture(GL_TEXTURE_2D, _textureId);
        glPixelStorei(GL_PACK_ALIGNMENT, getGLPixelAlignment(_format));
        glGetTexImage(GL_TEXTURE_2D, 0, textureFormatToGLFormat(_format, false), textureFormatToGLType(_format), buffer + offset);
        glBindTexture(GL_TEXTURE_2D, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);

        switch (_format)
        {
            case JCore::TextureFormat::Indexed8:
                glBindTexture(GL_TEXTURE_1D, _paletteId);
                glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
                glBindTexture(GL_TEXTURE_1D, 0);
                break;
            case JCore::TextureFormat::Indexed16:
                glBindTexture(GL_TEXTURE_2D, _paletteId);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
                glBindTexture(GL_TEXTURE_2D, 0);
                break;
        }
        return buffer;
    }

    void Texture::release() {
        releaseTexture();
        releasePalette();
        invalidate();
    }

    void Texture::releaseTexture() {
        if (_textureId) {
            glDeleteTextures(1, &_textureId);
            _textureId = 0;
        }
    }

    void Texture::releasePalette() {
        if (_paletteId) {
            glDeleteTextures(1, &_paletteId);
            _paletteId = 0;
        }
    }


    TextureGenState* getTextureGens() {
        static TextureGenState TEX_GEN_STATE[MAX_TEXTURES_QUEUED]{};
        return TEX_GEN_STATE;
    }

    int32_t getFirstAvailableTexGen() {
        auto gens = getTextureGens();
        for (int32_t i = 0; i < MAX_TEXTURES_QUEUED; i++) {
            if (gens[i].state == TEX_GEN_FREE) { return i; }
        }
        return -1;
    }

    void waitForTexGen(int32_t index, size_t sleepFor) {
        if (index < 0) { return; }
        while (shouldWaitForTexGen(index)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
        }
    }

    bool shouldWaitForTexGen(int32_t index) {
        if (index < 0) { return false; }
        auto& gen = getTextureGens()[index];
        switch (gen.state)
        {
            default: return true;
            case TEX_GEN_IDLE:
                gen.state = TEX_GEN_FREE;
                return false;
            case TEX_GEN_FREE:
                return false;
        }
    }

    int32_t setupTexGen(std::shared_ptr<Texture>& texture, const ImageData& data) {
        int32_t nextValid = getFirstAvailableTexGen();

        while (nextValid < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            nextValid = getFirstAvailableTexGen();
        }

        auto& genState = getTextureGens()[nextValid];
        genState.state = TEX_GEN_WAIT;
        genState.texture = &texture;
        genState.data = data;
        return nextValid;
    }

    void updateTexGen() {
        auto texGens = getTextureGens();
        for (size_t i = 0; i < MAX_TEXTURES_QUEUED; i++) {
            auto& genState = texGens[i];
            if (genState.state == TEX_GEN_WAIT) {
                auto& data = genState.data;
                genState.state = TEX_GEN_PROCESSING;

                if (genState.texture) {
                    std::shared_ptr<Texture>& tex = *genState.texture;
                    if (!tex) {
                        tex = std::make_shared<Texture>();
                    }

                    if (tex) {
                        tex->create(data.data, data.format, data.paletteSize, data.width, data.height, data.flags);
                    }
                }

                genState.state = TEX_GEN_IDLE;
                genState.texture = {};
                genState.data = {};
            }
        }
    }
}