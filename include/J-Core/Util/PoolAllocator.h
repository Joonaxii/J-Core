#pragma once
#include <cstdint>
#include <stdlib.h>
#include <J-Core/Math/Math.h>

namespace JCore {
    template<typename T>
    struct PoolChunk {
    public:
        static constexpr size_t CHUNK_SHIFT = 6;
        static constexpr size_t CHUNK_SIZE = 1 << CHUNK_SHIFT;
        PoolChunk<T>* next{ nullptr };

        const uint64_t getUsageMask() const { return _inUse; }
        T* getBuffer() { return _buffer; }
        const T* getBuffer() const { return _buffer; }

        constexpr bool isEmpty() const { return _inUse == 0; }
        constexpr bool isFull() const { return _inUse == UINT64_MAX; }
        constexpr bool hasFreeSlots() const { return _inUse != UINT64_MAX; }

        size_t countFreeSlots() const { return Math::countBits(_inUse); }
        void clear() { _inUse = 0; }

        T* allocateSlot() {
            if (isFull()) { return nullptr; }
            const int32_t newInd = Math::findFirstLSB(~_inUse);

            if (newInd < 0) { return nullptr; }
            _inUse |= (1ULL << newInd);
            return _buffer + newInd;
        }
        bool tryDeallocate(const T* ptr) {
            if (ptr < _buffer || ptr >= _buffer + CHUNK_SIZE) { return false; }
            _inUse &= ~(1ULL << int32_t(ptr - _buffer));
            return true;
        }

    private:
        uint64_t _inUse{};
        T _buffer[CHUNK_SIZE]{};
    };

    template<typename T, uint32_t init = 0>
    class PoolAllocator {
    public:
        PoolAllocator() : _chunk(nullptr), _chunkCount(0) {
            PoolChunk<T>* prev = _chunk;
            _chunkCount = init;
            for (size_t i = 0; i < init; i++) {
                PoolChunk<T>* chunk = new PoolChunk<T>();
                if (_chunk) {
                    prev->next = chunk;
                }
                else {
                    _chunk = chunk;
                }
                prev = chunk;
            }
        }
        ~PoolAllocator() {
            clear(true);
        }

        const size_t getChunkCount() const { return _chunkCount; }
        const PoolChunk<T>* getMainChunk() const { return _chunk; }

        static PoolAllocator<T, init>& getGlobal() { return Global; }

        void reserve(int64_t count) {
            PoolChunk<T>* prev = nullptr;
            PoolChunk<T>* cChnk = _chunk;
            while (cChnk) {
                count -= cChnk->countFreeSlots();
                prev = cChnk;
                cChnk = cChnk->next;
            }
            if (count <= 0) { return; }
            count = (count >> PoolChunk<T>::CHUNK_SHIFT) + 1;

            for (int64_t i = 0; i < count; i++) {
                cChnk = new PoolChunk<T>();
                _chunkCount++;
                if (prev) {
                    prev->next = cChnk;
                }
                else {
                    _chunk = cChnk;
                }
                prev = cChnk;
            }
        }

        T* allocate() {
            PoolChunk<T>* prev = nullptr;
            PoolChunk<T>* cChnk = _chunk;
            while (cChnk) {
                if (cChnk->hasFreeSlots()) {
                    break;
                }
                prev = cChnk;
                cChnk = cChnk->next;
            }

            if (!cChnk) {
                cChnk = new PoolChunk<T>();
                _chunkCount++;
                if (prev) {
                    prev->next = cChnk;
                }
                else {
                    _chunk = cChnk;
                }
            }
            return cChnk->allocateSlot();
        }

        template<class... Args>
        T* allocate(Args&&... args) {
            PoolChunk<T>* prev = nullptr;
            PoolChunk<T>* cChnk = _chunk;
            while (cChnk) {
                if (cChnk->hasFreeSlots()) {
                    break;
                }
                prev = cChnk;
                cChnk = cChnk->next;
            }

            if (!cChnk) {
                cChnk = new PoolChunk<T>();
                _chunkCount++;
                if (prev) {
                    prev->next = cChnk;
                }
                else {
                    _chunk = cChnk;
                }
            }
            T* ptrOut = cChnk->allocateSlot();
            if (!ptrOut) { return nullptr; }

            *ptrOut = T(args...);
            return ptrOut;
        }

        bool deallocate(T* obj) {
            if (obj) {
                PoolChunk<T>* ptr = _chunk;
                while (ptr) {
                    if (ptr->tryDeallocate(obj)) {
                        obj->~T();
                        return true;
                    }
                    ptr = ptr->next;
                }
            }
            return false;
        }

        void trim() {
            PoolChunk<T>* prev = nullptr;
            PoolChunk<T>* ptr = _chunk;
            while (ptr) {
                if (ptr->isEmpty()) {
                    if (prev) {
                        prev->next = ptr->next;
                    }
                    else {
                        _chunk = ptr->next;
                    }
                    auto temp = ptr;
                    ptr = ptr->next;
                    delete temp;
                    _chunkCount--;
                    continue;
                }
                prev = ptr;
                ptr = ptr->next;
            }
        }

        void clear(bool full) {
            PoolChunk<T>* ptr = _chunk;
            while (ptr) {
                PoolChunk<T>* temp = ptr;
                ptr = ptr->next;
                uint64_t chunkMsk = temp->getUsageMask();
                if (chunkMsk) {
                    T* buffer = temp->getBuffer();
                    for (uint64_t i = 0, j = 1; i < PoolChunk<T>::CHUNK_SIZE; i++, j <<= 1) {
                        if (chunkMsk & j) {
                            (buffer + i)->~T();
                        }
                    }
                    temp->clear();
                }

                if (full) {
                    delete temp;
                }
            }

            if (full) {
                _chunk = nullptr;
                _chunkCount = 0;
            }
        }

    private:
        static PoolAllocator<T, init> Global;
        size_t _chunkCount;
        PoolChunk<T>* _chunk;
        
    };

    template<typename T, uint32_t init>
    PoolAllocator<T, init> PoolAllocator<T, init>::Global{};

    template<typename T, size_t init = 0>
    void clearPoolAllocator(bool full) {
        PoolAllocator<T, init>::getGlobal().clear(full);
    }

    template<typename T, size_t init = 0>
    void trimPoolAllocator() {
        PoolAllocator<T, init>::getGlobal().trim();
    }
}