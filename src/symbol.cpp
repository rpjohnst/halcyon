#include "symbol.h"
#include "arena.h"

Symbol::Symbol(uint32_t uniq, std::string_view text) : uniq(uniq), len(text.size()) {
    std::copy(text.begin(), text.end(), reinterpret_cast<char *>(this + 1));
}

auto Symbol::intern(Arena &arena, Intern &intern, std::string_view text) -> Symbol * {
    auto it = intern.find(text);
    if (it != intern.end()) { return *it; }

    auto p = arena.alloc(sizeof(Symbol) + text.size(), alignof(Symbol));
    return *intern.emplace_hint(it, new (p) Symbol{uint32_t(intern.size()), text});
}
