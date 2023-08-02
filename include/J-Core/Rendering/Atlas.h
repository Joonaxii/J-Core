#pragma once
#include <memory>
#include <vector>
#include <algorithm>
#include <J-Core/Rendering/Sprite.h>
#include <unordered_map>
#include <J-Core/IO/ImageUtils.h>
#include <J-Core/Util/PoolAllocator.h>
#include <J-Core/Util/Stack.h>

namespace JCore {
    template<typename T>
    struct TexturePackingSpecs {
        const T* sprites{ nullptr };
        size_t spriteCount{ 0 };
        bool sort{ true };
        uint16_t maxSize{ 8192 };
        uint8_t padding{ 0 };
        size_t poolInit{0};
        size_t stackInit{0};
    };

    struct TextureRegion {
        uint32_t original{ 0 };
        SpriteRect rect{};

        TextureRegion() : original(0), rect() {}
        TextureRegion(uint32_t orignal, SpriteRect rect) : original(orignal), rect(rect) {}
    };

    struct AtlasDefiniton {
        uint16_t width{ 0 };
        uint16_t height{ 0 };
        std::vector<TextureRegion> atlas{};

        AtlasDefiniton() : width(), height(), atlas{} {}
    };

    template<typename T>
    bool sortSprites(const T* a, const T* b) {
        const int32_t resoA(a->getWidth() * a->getHeight());
        const int32_t resoB(b->getWidth() * b->getHeight());
        return resoA > resoB;
    }

    template<typename T>
    uint32_t indexOfSprite(const TexturePackingSpecs<T>& specs, const T* value) {
        return value ? uint32_t(value - specs.sprites) : 0;
    }

    template<typename T>
    bool packSprites(const TexturePackingSpecs<T>& specs, std::vector<AtlasDefiniton>& results) {
        struct PackingNode {
            const T* sprite{};
            SpriteRect rect{};
            bool notLeaf{ 0 };
            PackingNode* children[2]{ nullptr };

            PackingNode() : sprite{ nullptr }, rect{}, notLeaf{0}, children{} {}
            ~PackingNode() { clear(); }

            static PoolAllocator<PackingNode>& getAllocator() {
                static PoolAllocator<PackingNode> allocator{};
                return allocator;
            }

            void clear() {
                sprite = nullptr;
                notLeaf = 0;

                if (children[0]) {
                    getAllocator().deallocate(children[0]);
                    children[0] = nullptr;
                }

                if (children[1]) {
                    getAllocator().deallocate(children[1]);
                    children[1] = nullptr;
                }
            }

            void setup(uint16_t width, uint16_t height) {
                rect = { 0, 0, width, height };
            }

            PackingNode* insert(Stack<PackingNode*>& nodeStack, const T* sprite, const int32_t padding) {
                nodeStack.clear();
                nodeStack.push(this);

                PackingNode* select = nullptr;
                while (nodeStack.hasItems()) {
                    PackingNode* current = nodeStack.top();
                    nodeStack.pop();

                    if (current->notLeaf) {
                        nodeStack.push(current->children[1]);
                        nodeStack.push(current->children[0]);
                        continue;
                    }

                    if (current->sprite) { continue; }

                    auto& rect = current->rect;

                    uint16_t pW = sprite->getWidth() + padding;
                    uint16_t pH = sprite->getHeight() + padding;

                    if (rect.width < pW || rect.height < pH) { continue; }
                    if (rect.width == pW && rect.height == pH) { 
                        return current; 
                    }

                    current->children[0] = getAllocator().allocate();
                    current->children[1] = getAllocator().allocate();

                    current->notLeaf |= 0x3;

                    PackingNode& ch0 = *current->children[0];
                    PackingNode& ch1 = *current->children[1];

                    uint16_t dW = rect.width - pW;
                    uint16_t dH = rect.height - pH;

                    if (dW > dH) {
                        ch0.rect = { rect.x,                rect.y, pW, rect.height };
                        ch1.rect = { uint16_t(rect.x + pW), rect.y, dW, rect.height };
                    }
                    else {
                        ch0.rect = { rect.x, rect.y,                rect.width, pH };
                        ch1.rect = { rect.x, uint16_t(rect.y + pH), rect.width, dH };
                    }
                    nodeStack.push(&ch0);
                }
                return nullptr;
            }
        };

        PackingNode::getAllocator().reserve(specs.poolInit);
        Stack<PackingNode*> nodeStack{};
        nodeStack.reserve(specs.stackInit);

        std::vector<const T*> temp;
        temp.reserve(specs.spriteCount);
        for (size_t i = 0; i < specs.spriteCount; i++) {
            temp.push_back(&specs.sprites[i]);
        }

        if (specs.sort) {
            std::sort(temp.begin(), temp.end(), sortSprites<T>);
        }

        AtlasDefiniton res{};

        res.atlas.reserve(specs.spriteCount);
        res.width = 16;
        res.height = 16;

        uint32_t spriteFloor = 0;
        PackingNode mainNode{};

        while (spriteFloor < temp.size()) {
            if (res.width > specs.maxSize || res.height > specs.maxSize) {
                res.width = std::min(res.width, specs.maxSize);
                res.height = std::min(res.height, specs.maxSize);
                results.push_back(res);

                spriteFloor += uint32_t(res.atlas.size());

                res.width = 16;
                res.height = 16;
            }
            res.atlas.clear();
            mainNode.setup(res.width, res.height);

            bool success = true;
            for (size_t i = spriteFloor; i < temp.size(); i++) {
                PackingNode* node = mainNode.insert(nodeStack, temp[i], specs.padding);
                if (!node || node->sprite) { success = false; break; }
                node->sprite = temp[i];
                res.atlas.emplace_back(indexOfSprite(specs, node->sprite), node->rect);
            }

            mainNode.clear();
            if (success) { break; }
            if (res.width > res.height) {
                res.height <<= 1;
            }
            else { res.width <<= 1; }
        }

        if (res.atlas.size() > 0) {
            res.width = std::min(res.width, specs.maxSize);
            res.height = std::min(res.height, specs.maxSize);
            results.push_back(res);
        }
        return true;
    }

    template<int32_t fWidth, int32_t fHeight, int32_t padding = 0>
    bool packSpritesFixed(int32_t maxSize, size_t spriteCount, std::vector<AtlasDefiniton>& results) {
        AtlasDefiniton res{};

        static constexpr int32_t WIDTH_PAD = (fWidth + padding);
        static constexpr int32_t HEIGHT_PAD = (fHeight + padding);
        static constexpr int32_t PAD_RESO = (WIDTH_PAD * HEIGHT_PAD);

        int32_t minX = 16;
        int32_t minY = 16;

        int32_t maxX = (maxSize / WIDTH_PAD);
        int32_t maxY = (maxSize / HEIGHT_PAD);

        while (true) {
            bool changed = false;
            if (minX < fWidth && minX < maxSize) {
                minX <<= 1;
                changed = true;
            }

            if (minY < fHeight && minY < maxSize) {
                minY <<= 1;
                changed = true;
            }
            if (!changed) { break; }
        }

        res.atlas.reserve(maxX * maxY);
        res.width = minX;
        res.height = minX;

        uint32_t indices = 0;
        uint32_t totalLeft = spriteCount;
        while (totalLeft > 0) {
            int32_t itemReso = PAD_RESO * (totalLeft);

            int32_t reso = (res.width * res.height);
            while (reso < itemReso) {
                if (res.width > res.height) {
                    res.height <<= 1;
                }
                else { res.width <<= 1; }
                if (res.width > maxSize || res.height > maxSize) {
                    res.width = uint16_t(std::min<int32_t>(res.width, maxSize));
                    res.height = uint16_t(std::min<int32_t>(res.height, maxSize));
                    break;
                }
                reso = (res.width * res.height);
            }

            int32_t cX = res.width / WIDTH_PAD;
            int32_t cY = res.height / HEIGHT_PAD;

            for (size_t y = 0; y < res.height; y += HEIGHT_PAD) {
                for (size_t x = 0; x < res.width; x += WIDTH_PAD) {
                    res.atlas.emplace_back(indices++, SpriteRect(x, y, fWidth, fHeight));
                    if (indices >= spriteCount) { goto end; }
                }
            }

            end:
            results.push_back(res);
            totalLeft -= uint32_t(res.atlas.size());

            res.width = minX;
            res.height = minY;

            res.atlas.clear();
        }
        return true;
    }


    class Texture;
    class Atlas {
    public:
        Atlas();
        Atlas(const SpriteInfo* frames, size_t frameCount, const ImageData& imageData);
        ~Atlas();

        void setSprites(const SpriteInfo* frames, size_t frameCount);
        void applyPixelData(const ImageData& imageData);

        std::weak_ptr<Texture> getTexture() const { return _texture; }
        const Sprite* findByName(const std::string& str) const;

        bool isValid() const;

    private:
        std::shared_ptr<Texture> _texture;
        std::vector<Sprite> _sprites;
        std::unordered_map<std::string, int32_t> _nameToIndex;
    };

}