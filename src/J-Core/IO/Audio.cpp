#include <J-Core/IO/Audio.h>
#include <J-Core/IO/FileStream.h>
#include <J-Core/Log.h>

namespace JCore {

    namespace Wav {
        struct Header {
            char riff[4]{ 'R', 'I', 'F', 'F' };
            uint32_t size{ 0 };
            char wave[4]{ 'W', 'A', 'V', 'E' };
        };

        struct FormatChunk {
            char fmt[4]{0};
            uint32_t size{ 0 };
            uint16_t type{ 0 };
            uint16_t channels{ 0 };
            uint32_t sampleRate{ 0 };
            uint32_t byteRate{ 0 };
            uint16_t blockAlign{ 0 };
            uint16_t bitsPerSample{ 0 };
        };

        bool getInfo(std::string_view path, AudioData& audio) {
            FileStream fs(path, "rb");
            if (fs.isOpen()) {
                return getInfo(fs, audio);
            }
            JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to open file '{0}' for reading!", path);
            return false;
        }

        bool getInfo(const Stream& stream, AudioData& audio) {
            size_t pos = stream.tell();

            Header header{};
            size_t read = stream.readValue(header, false);

            if (read < 1 ||
                strncmp(header.riff, "RIFF", 4) != 0 ||
                strncmp(header.wave, "WAVE", 4) != 0 ||
                header.size <= 36) {
                JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to decode wav, not a WAVE file!");
                return false;
            }
            FormatChunk fmtCh{};
            stream.readValue(fmtCh, false);

            if (strncmp(fmtCh.fmt, "fmt ", 4) != 0) {
                JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to decode wav, SubChunk 1 is not where it should be!");
                return false;
            }

            if (fmtCh.type != 1) {
                JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to decode wav, PCM is only supported but found type '{0}'!", fmtCh.type);
                return false;
            }

            //We seek to the start of SubChunk 2, just in case there is data we don't need in SubChunk1
            stream.seek(pos + fmtCh.size + 8 + sizeof(Header), SEEK_SET);

            char temp[5]{ 0 };
            stream.read(temp, 4, false);

            if (strncmp(temp, "data", 4) != 0) {
                if (strncmp(temp, "LIST", 4) != 0) {
                    JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to decode wav, SubChunk 2 is not where it should be!");
                    return false;
                }
                stream.seek(stream.readValue<uint32_t>(), SEEK_CUR);
                stream.read(temp, 4, false);
                if (strncmp(temp, "data", 4) != 0) {
                    JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to decode wav, SubChunk 2 is not where it should be!");
                    return false;
                }
            }

            uint32_t size{ 0 };
            stream.readValue(size, false);

            audio.clear(false);
            audio.format = AudioFormat::PCM;
            audio.sampleRate = fmtCh.sampleRate;
            audio.depth = uint8_t(fmtCh.bitsPerSample);
            audio.channels = uint8_t(fmtCh.channels);
            audio.blockAlign = fmtCh.blockAlign;
            audio.sampleType = audio.depth == 8 ? AudioSampleType::Unsigned : AudioSampleType::Signed;
            audio.sampleCount = (size / ((fmtCh.bitsPerSample >> 3) * (fmtCh.channels)));
            return true;
        }

        bool decode(std::string_view path, AudioData& audio) {
            FileStream fs(path, "rb");
            if (fs.isOpen()) {
                return decode(fs, audio);
            }
            JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to open file '{0}' for reading!", path);
            return false;
        }

        bool decode(const Stream& stream, AudioData& audio) {
            size_t pos = stream.tell();

            if (!getInfo(stream, audio)) {
                return false;
            }

            if (!audio.doAllocate()) {
                JCORE_ERROR("[Audio-IO] (WAV) Decode Error: Failed to allocate audio data buffer!");
                return false;
            }
            stream.read(audio.data, audio.getBufferSize(), false);
            return true;
        }


        bool encode(std::string_view path, const AudioData& audio) {
            FileStream fs(path, "wb");
            if (fs.isOpen()) {
                return encode(fs, audio);
            }
            JCORE_ERROR("[Audio-IO] (WAV) Encode Error: Failed to open file '{0}' for writing!", path);
            return false;
        }

        bool encode(const Stream& stream, const AudioData& audio) {
            JCORE_ERROR("[Audio-IO] (WAV) Encode Error: Not Implenented!");
            return false;
        }
    }
}