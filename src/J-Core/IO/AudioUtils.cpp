#include <J-Core/IO/AudioUtils.h>
#include <algorithm>

namespace JCore {

    namespace Audio {
        float sampleU8ToF_T(uint8_t sample) {
            static float TABLE[UINT8_MAX + 1]{ 0 };
            static bool init{ false };

            if (!init) {
                for (uint32_t i = 0; i <= UINT8_MAX; i++) {
                    TABLE[i] = fromSampleU8F(uint8_t(i));
                }
                init = true;
            }

            return TABLE[sample];
        }

        float sampleI8ToF_T(int8_t sample) {
            static float TABLE[UINT8_MAX + 1]{ 0 };
            static bool init{ false };

            if (!init) {
                for (int32_t i = INT8_MIN, j = 0; i <= INT8_MAX; i++) {
                    TABLE[j++] = fromSampleI8F(int8_t(i));
                }
                init = true;
            }

            return TABLE[sample - INT16_MIN];
        }


        float sampleU16ToF_T(uint16_t sample) {
            static float TABLE[UINT16_MAX + 1]{ 0 };
            static bool init{ false };

            if (!init) {
                for (uint32_t i = 0; i <= UINT16_MAX; i++) {
                    TABLE[i] = fromSampleU16F(uint16_t(i));
                }
                init = true;
            }

            return TABLE[sample];
        }

        float sampleI16ToF_T(int16_t sample) {
            static float TABLE[UINT16_MAX + 1]{ 0 };
            static bool init{ false };

            if (!init) {
                for (int32_t i = INT16_MIN, j = 0; i <= INT16_MAX; i++) {
                    TABLE[j++] = fromSampleI16F(int16_t(i));
                }
                init = true;
            }

            return TABLE[sample - INT16_MIN];
        }
    }
}