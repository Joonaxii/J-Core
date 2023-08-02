#include <J-Core/Rendering/Buffers/FrameBufferPool.h>
#include <J-Core/Log.h>

namespace JCore {
    FrameBufferPool::FrameBufferPool() : FrameBufferPool(16) {}

    FrameBufferPool::FrameBufferPool(size_t initCapacity) : _buffers(initCapacity) {}

    void FrameBufferPool::tick() {
        for (int32_t i = 0; i < _buffers.size(); i++) {
            auto& buf = _buffers[i];
            if (buf.ticksLeft > 0 && buf.id != 0) {
                buf.ticksLeft--;
                if (buf.ticksLeft == 0) {
                    _bufferLut.erase(buf.id);

                    JCORE_TRACE("[J-Core - Frame Buffer Pool] Reset frame buffer 0x{0:X}!", buf.id);
                    buf.id = 0;
                    buf.buffer.releaseColorAttatchment();
                    buf.buffer.releaseBuffer();
                }
            }
        }
    }

    const FrameBuffer* FrameBufferPool::retrieve(uint64_t id, bool& newInstance) {
        if (id == 0) {
            JCORE_WARN("[J-Core - Frame Buffer Pool] Warning: Id of 0 cannot be used!");
            return nullptr;
        }

        auto find = _bufferLut.find(id);
        if (find != _bufferLut.end()) {
            int32_t ind = find->second;
            _buffers[ind].id = id;
            _buffers[ind].ticksLeft = FB_POOL_MAX_TICKS_ACTIVE;
            newInstance = false;
            return &_buffers[ind].buffer;
        }

        newInstance = true;
        for (int32_t i = 0; i < _buffers.size(); i++) {
            auto& buf = _buffers[i];
            if (buf.ticksLeft == 0 || buf.id == 0) {
                _bufferLut.insert(std::make_pair(id, i));
                _buffers[i].ticksLeft = id;
                _buffers[i].ticksLeft = FB_POOL_MAX_TICKS_ACTIVE;
                return &_buffers[i].buffer;
            }
        }

        _bufferLut.insert(std::make_pair(id, int32_t(_buffers.size())));
        auto& bufOut = _buffers.emplace_back();
        bufOut.id = id;
        bufOut.ticksLeft = FB_POOL_MAX_TICKS_ACTIVE;
        return &bufOut.buffer;
    }
}