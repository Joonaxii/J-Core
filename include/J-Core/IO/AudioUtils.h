#pragma once
#include <cstring>
#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <J-Core/Util/EnumUtils.h>
#include <J-Core/Math/Math.h>

namespace JCore {
    enum AudioDataFlags : uint8_t {
        AUD_FLAG_NONE = 0x0,
        AUD_FLAG_OWNS_BUFFER = 0x1,
    };

    enum class AudioFormat : uint8_t {
        PCM,
        OGG
    };

    enum class AudioSampleType : uint8_t {
        Signed,
        Unsigned,
        Float,
    };
}

namespace JCore {

    struct AudioData {
    public:
        AudioDataFlags flags{};
        AudioFormat format{};
        AudioSampleType sampleType{};
        uint8_t depth{};
        uint8_t channels{};
        uint32_t sampleRate{};
        uint16_t blockAlign{};
        size_t sampleCount{};
        uint8_t* data{};

        constexpr AudioData() : format(), sampleType(), depth(), channels(), sampleRate(), sampleCount(), blockAlign(), data(nullptr) { }
        constexpr AudioData(AudioFormat format, AudioSampleType type, uint8_t depth, uint8_t channels, uint32_t frequency, size_t size) :
            format(format), sampleType(type), depth(depth), channels(), sampleRate(), sampleCount(), blockAlign(), data(nullptr) { }

        ~AudioData() {
            clear(flags & AUD_FLAG_OWNS_BUFFER);
        }

        constexpr size_t getBufferSize() const { return _bufferSize; }

        constexpr size_t calculateSize() const { return calculateSize(format, sampleType, depth, channels, sampleCount); }

        template<typename T>
        constexpr T getLength() const {
            return T(sampleCount) / T(Math::max<uint32_t>(sampleRate, 1U));
        }

        static constexpr size_t calculateSize(AudioFormat format, AudioSampleType type, uint8_t bitDepth, uint8_t channels, size_t sampleCount) {
            switch (format)
            {
                case AudioFormat::PCM:
                    return sampleCount * channels * (bitDepth >> 3);
                default: return 0;
            }
        }

        bool doAllocate() {
            return doAllocate(calculateSize());
        }

        bool doAllocate(size_t size) {
            if (size < _bufferSize && data) { return true; }

            if (data) {
                if (flags & AUD_FLAG_OWNS_BUFFER) {
                    uint8_t* temp = reinterpret_cast<uint8_t*>(realloc(data, size));
                    if (temp == nullptr) {
                        return false;
                    }
                    memset(temp + _bufferSize, 0, size - _bufferSize);
                    data = temp;
                    _bufferSize = size;
                    return true;
                }
            }
            data = reinterpret_cast<uint8_t*>(malloc(size));
            flags =  AudioDataFlags(flags | AUD_FLAG_OWNS_BUFFER);
            _bufferSize = size;
            return data != nullptr;
        }
        
        bool doAllocate(AudioFormat format, AudioSampleType type, uint8_t bitDepth, uint8_t channels, size_t sampleCount, bool modify = true) {
            if (modify) {
                this->format = format;
                this->sampleType = type;
                this->depth = bitDepth;
                this->channels = channels;
                this->sampleCount = sampleCount;
            }
            return doAllocate(calculateSize(format, type, bitDepth, channels, sampleCount));
        }

        void release() {
            if (data && (flags & AUD_FLAG_OWNS_BUFFER)) {
                free(data);
            }
            data = nullptr;
        }

        void clear(bool destroy) {
            if (destroy && data) {
                release();
            }
            _bufferSize = 0;

            flags = {};
            format = { };
            sampleType = { };
            depth = 0;
            channels = 0;
            sampleRate = 0;
            blockAlign = 0;
            sampleCount = 0;
        }

    private:
        size_t _bufferSize{ 0 };
    };

    namespace Audio {
        template<uint32_t sampleRate, typename P = float>
        inline constexpr float sampleToSeconds(int64_t sample) {
            return P(sample) * (P() / P(sampleRate));
        }

        template<typename P = float>
        inline constexpr P sampleToSeconds(int64_t sample, uint32_t sampleRate) {
            return P(sample) / P(sampleRate);
        }

        template<typename P = float>
        inline constexpr int64_t secondsToSample(P seconds, uint32_t sampleRate) {
            return int64_t(seconds * sampleRate);
        }


        template<typename P = float>
        inline constexpr P getPanR(P pan) {
            return pan < 0 ? (P(1) + pan) : P(1);
        }

        template<typename P = float>
        inline constexpr P getPanL(P pan) {
            return pan > 0 ? (P(1) - pan) : P(1);
        }

        //To Sample
        inline constexpr int8_t toSampleI8F(float wave) {
            if (wave < 0) {
                return int8_t((wave < -1.0f ? -1.0f : wave) * -INT8_MIN);
            }
            return int8_t((wave > 1.0f ? 1.0f : wave) * INT8_MAX);
        }

        inline constexpr uint8_t toSampleU8F(float wave) {
            return int8_t((wave > 1.0f ? 1.0f : wave) * UINT8_MAX);
        }

        inline constexpr int16_t toSampleI16F(float wave) {
            if (wave < 0) {
                return int16_t((wave < -1.0f ? -1.0f : wave) * -INT16_MIN);
            }
            return int16_t((wave > 1.0f ? 1.0f : wave) * INT16_MAX);
        }

        inline constexpr uint16_t toSampleU16F(float wave) {
            return int16_t((wave > 1.0f ? 1.0f : wave) * UINT16_MAX);
        }

        inline constexpr int32_t toSampleI32F(float wave) {
            if (wave < 0) {
                return int32_t((wave < -1.0f ? -1.0f : wave) * -INT32_MIN);
            }
            return int32_t((wave > 1.0f ? 1.0f : wave) * INT32_MAX);
        }

        inline constexpr uint32_t toSampleU32F(float wave) {
            return int32_t((wave > 1.0f ? 1.0f : wave) * UINT32_MAX);
        }



        inline constexpr int8_t toSampleI8D(double wave) {
            if (wave < 0) {
                return int8_t((wave < -1.0f ? -1.0f : wave) * -INT8_MIN);
            }
            return int8_t((wave > 1.0f ? 1.0f : wave) * INT8_MAX);
        }

        inline constexpr uint8_t toSampleU8D(double wave) {
            return int8_t((wave > 1.0f ? 1.0f : wave) * UINT8_MAX);
        }

        inline constexpr int16_t toSampleI16D(double wave) {
            if (wave < 0) {
                return int16_t((wave < -1.0f ? -1.0f : wave) * -INT16_MIN);
            }
            return int16_t((wave > 1.0f ? 1.0f : wave) * INT16_MAX);
        }

        inline constexpr uint16_t toSampleU16D(double wave) {
            return int16_t((wave > 1.0f ? 1.0f : wave) * UINT16_MAX);
        }

        inline constexpr int32_t toSampleI32D(double wave) {
            if (wave < 0) {
                return int32_t((wave < -1.0f ? -1.0f : wave) * -INT32_MIN);
            }
            return int32_t((wave > 1.0f ? 1.0f : wave) * INT32_MAX);
        }

        inline constexpr uint32_t toSampleU32D(double wave) {
            return int32_t((wave > 1.0f ? 1.0f : wave) * UINT32_MAX);
        }


        //From Sample
        inline constexpr float fromSampleI8F(int8_t wave) {
            return wave < 0 ? float(wave) / INT8_MIN : float(wave) / INT8_MAX;
        }

        inline constexpr float fromSampleU8F(uint8_t wave) {
            return float(wave) / UINT8_MAX;
        }

        inline constexpr float fromSampleI16F(int16_t wave) {
            return wave < 0 ? float(wave) / INT16_MIN : float(wave) / INT16_MAX;
        }

        inline constexpr float fromSampleU16F(uint16_t wave) {
            return float(wave) / UINT16_MAX;
        }

        inline constexpr float fromSampleI32F(int32_t wave) {
            return wave < 0 ? float(wave) / INT32_MIN : float(wave) / INT32_MAX;
        }

        inline constexpr float fromSampleU32F(uint32_t wave) {
            return float(wave) / UINT32_MAX;
        }

        float sampleU8ToF_T(uint8_t sample);
        float sampleI8ToF_T(int8_t sample);

        float sampleU16ToF_T(uint16_t sample);
        float sampleI16ToF_T(int16_t sample);

        inline constexpr double fromSampleI8D(int8_t wave) {
            return wave < 0 ? double(wave) / INT8_MIN : double(wave) / INT8_MAX;
        }

        inline constexpr double fromSampleU8D(uint8_t wave) {
            return double(wave) / UINT8_MAX;
        }

        inline constexpr double fromSampleI16D(int16_t wave) {
            return wave < 0 ? double(wave) / INT16_MIN : double(wave) / INT16_MAX;
        }

        inline constexpr double fromSampleU16D(uint16_t wave) {
            return double(wave) / UINT16_MAX;
        }

        inline constexpr double fromSampleI32D(int32_t wave) {
            return wave < 0 ? double(wave) / INT32_MIN : double(wave) / INT32_MAX;
        }

        inline constexpr double fromSampleU32D(uint32_t wave) {
            return double(wave) / UINT32_MAX;
        }
    }
}
