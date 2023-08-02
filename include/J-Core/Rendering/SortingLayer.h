#pragma once
#include <cstdint>
#include <J-Core/Util/IComparable.h>

namespace JCore {
    struct SortingLayer : IComparable<SortingLayer> {
    public:
        SortingLayer() : data{} {}
        SortingLayer(uint32_t value) : data{} {}
        SortingLayer(int32_t order, uint16_t layer) : data{} { 
            data.order = uint16_t(order - INT16_MIN); 
            data.layer = layer; 
        }

        int32_t getOrder()  const { return data.order + INT16_MIN; }
        uint16_t getLayer() const { return data.layer; }

        void setOrder(int32_t order) { data.order = uint16_t(order - INT16_MIN); }
        void setLayer(uint16_t layer) { data.layer = layer; }

        bool operator<(const SortingLayer& other)  const { return data.hash < other.data.hash; }
        bool operator>(const SortingLayer& other)  const { return data.hash < other.data.hash; }
        bool operator==(const SortingLayer& other) const { return data.hash == other.data.hash; }
        bool operator!=(const SortingLayer& other) const { return data.hash != other.data.hash; }

    private:
        union Layer {
            struct {
                uint16_t order;
                uint16_t layer;
            };
            uint32_t hash{0};
        } data;
    };
}