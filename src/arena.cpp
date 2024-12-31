#include "arena.h"

static constexpr size_t SLAB_SIZE = 0x1000;

auto Arena::alloc_next_slab(size_t size, size_t align) -> void* {
    // Put large allocations in their own slabs.
    auto padded = size + align - 1;
    if (SLAB_SIZE < padded) {
        auto slab = this->alloc_slab(padded);
        return std::align(align, size, slab, padded);
    }

    // Double slab size every 128 slabs up to 2GB.
    auto len = SLAB_SIZE * (1 << std::min(30zu, this->slabs.size() / 128));
    this->next = this->alloc_slab(len);
    this->len = len;

    return this->alloc(size, align);
}

auto Arena::alloc_slab(size_t size) -> void * {
    return this->slabs.emplace_back(std::make_unique<std::byte []>(size)).get();
}
