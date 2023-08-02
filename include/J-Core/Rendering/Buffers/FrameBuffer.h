#pragma once
#include <cstdint>
#include <J-Core/Math/Matrix4f.h>

namespace JCore {
    struct FrameBufferSpecs {
        uint32_t width{0};
        uint32_t height{0};
        uint32_t colorFormat{0};
        uint8_t* pixelData{nullptr};

        bool operator==(const FrameBufferSpecs& other) const {
            return width == other.width && height == other.height && colorFormat == other.colorFormat && pixelData == other.pixelData;
        }

        bool operator!=(const FrameBufferSpecs& other) const {
            return !(*this == other);
        }
    };

    class FrameBuffer {
    public:

        FrameBuffer();
        FrameBuffer(FrameBuffer&& other) noexcept;

        FrameBuffer(const uint32_t colorFormat);
        FrameBuffer(const FrameBufferSpecs& specs);
        FrameBuffer(const uint32_t colorFormat, const FrameBufferSpecs& specs);
        ~FrameBuffer() noexcept;

        const uint32_t& getBufferId() const { return _bufferId; }
        const uint32_t getColorAttatchment() const { return _colorAttatchment; }

        const FrameBufferSpecs& getSpecs() const { return _specs; }
        const Matrix4f& getProjectionMatrix() const { return _projMatrix; }

        void refresh() const;

        bool isValid() const { return bool(_colorAttatchment) && bool(_bufferId); }

        void invalidate() const;
        void invalidate(const FrameBufferSpecs& specs, const bool instant = true, const bool force = false) const;
        void releaseBuffer() const noexcept;

        void releaseColorAttatchment() const noexcept;
    
        uint32_t bindColorAttachment(const uint32_t slot) const;
        uint32_t unbindColorAttachment(const uint32_t slot) const;

        void blit(const uint8_t* data);
        void blit(const uint8_t* data, const int32_t x, const int32_t y, const int32_t width, const int32_t height, const uint32_t format);

        bool bind() const;
        void unbind() const;

    private:
        mutable FrameBufferSpecs _specs;
        mutable uint32_t _bufferId;

        mutable uint32_t _colorAttatchment;
        mutable Matrix4f _projMatrix;

        mutable bool _dirty;

        void invalidate(const bool rebuild) const;
    };
}