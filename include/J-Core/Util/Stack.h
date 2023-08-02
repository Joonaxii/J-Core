#pragma once
#include <cstdint>
#include <stdlib.h>

namespace JCore {
    template<typename T>
    class Stack {
    public:
        Stack() : Stack(8) {}
        Stack(size_t capacity) : _tail(0), _capacity(capacity), _buffer(nullptr) {
            reserve(capacity);
        }

        ~Stack() {
            release();
        }

        T top() const {
            return _buffer && _tail > 0 ? _buffer[_tail - 1] : T();
        }

        constexpr bool hasItems(size_t min = 0) const{
            return _tail > min;
        }

        void clear() {
            _tail = 0;
        }

        bool pop() {
            if (_tail < 1) { return false; }
            _tail--;
            return true;
        }

        void push(const T& value) {
            if (_tail >= _capacity) {
                size_t newCap = std::max<size_t>(_capacity, 1);
                while (newCap < _tail) {
                    newCap <<= 1;
                }
                reserve(newCap);
            }
            _buffer[_tail++] = value;
        }

        void release() {
            if (_buffer) {
                free(_buffer);
                _capacity = 0;
                _tail = 0;
                _buffer = nullptr;
            }
        }

        void reserve(size_t newCap) {
            if (_buffer) {
                void* reall = realloc(_buffer, newCap * sizeof(T));

                if (reall) {
                    _buffer = reinterpret_cast<T*>(reall);
                    _capacity = newCap;
                }
                return;
            }
            _buffer = reinterpret_cast<T*>(malloc(sizeof(T) * newCap));
            _capacity = newCap;
        }

    private:
        size_t _capacity;
        size_t _tail;
        T* _buffer;
    };


}