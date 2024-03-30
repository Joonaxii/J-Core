#pragma once
#include <J-Core/IO/AudioUtils.h>
#include <J-Core/IO/Stream.h>

namespace JCore {

    namespace Wav {
        bool getInfo(std::string_view path, AudioData& audio);
        bool getInfo(const Stream& stream, AudioData& audio);

        bool decode(std::string_view path, AudioData& audio);
        bool decode(const Stream& stream, AudioData& audio);

        bool encode(std::string_view path, const AudioData& audio);
        bool encode(const Stream& stream, const AudioData& audio);

    }
}