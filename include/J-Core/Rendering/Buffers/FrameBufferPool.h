#pragma once
#include <cstdint>
#include <vector>
#include <J-Core/Rendering/Buffers/FrameBuffer.h>
#include <deque>
#include <unordered_map>
static constexpr uint64_t FB_POOL_MAX_TICKS_ACTIVE = 120;

namespace JCore {

    class FrameBufferPool {
    public:
        FrameBufferPool();
        FrameBufferPool(size_t initCapacity);

        void tick();
        const FrameBuffer* retrieve(uint64_t id, bool& newInstance);

    private:
        struct PoolInstance {
            uint64_t id{};
            uint64_t ticksLeft{};
            int32_t bufferIndex{};
            FrameBuffer buffer{};
        };

        std::unordered_map<uint64_t, int32_t> _bufferLut;
        std::vector<PoolInstance> _buffers;
    };
}