#ifndef ARENA_H
#define ARENA_H

#include <vector>
#include <memory>

class Arena {
    std::vector<std::unique_ptr<std::byte []>> slabs;
    void *next = nullptr;
    size_t len = 0;

public:
    template <class T, class ...As>
    auto alloc(As &&...as) -> T *;

    auto alloc(size_t size, size_t align) -> void *;

private:
    auto alloc_next_slab(size_t size, size_t align) -> void *;
    auto alloc_slab(size_t size) -> void *;
};

template <class T, class ...As>
auto Arena::alloc(As &&...as) -> T * {
    return new (this->alloc(sizeof(T), alignof(T))) T{std::forward<As>(as)...};
}

inline auto Arena::alloc(size_t size, size_t align) -> void * {
    if (std::align(align, size, this->next, this->len) == nullptr) {
        return this->alloc_next_slab(size, align);
    }

    void* data = this->next;
    this->next = static_cast<std::byte *>(this->next) + size;
    this->len -= size;
    return data;
}

#endif
