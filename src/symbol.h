#ifndef SYMBOL_H
#define SYMBOL_H

#include <unordered_set>
#include <string_view>
#include <cstdint>
#include <cstddef>

struct Arena;

struct Symbol {
    uint32_t uniq;
    size_t len;

    operator std::string_view() const;

private:
    Symbol(uint32_t uniq, std::string_view text);

    struct SymbolHash {
        using is_transparent = void;
        auto operator()(Symbol *symbol) const -> size_t { return operator()(*symbol); }
        auto operator()(std::string_view text) const -> size_t {
            return std::hash<std::string_view>{}(text);
        }
    };
    struct SymbolEq {
        using is_transparent = void;
        auto operator()(Symbol *a, Symbol *b) const -> bool { return operator()(*a, *b); }
        auto operator()(Symbol *a, std::string_view b) const -> bool { return operator()(*a, b); }
        auto operator()(std::string_view a, Symbol *b) const -> bool { return operator()(a, *b); }
        auto operator()(std::string_view a, std::string_view b) const -> bool {
            return std::equal_to<std::string_view>{}(a, b);
        }
    };

public:
    using Intern = std::unordered_set<Symbol *, SymbolHash, SymbolEq>;
    static auto intern(Arena &arena, Intern &intern, std::string_view text) -> Symbol *;
};

inline Symbol::operator std::string_view() const {
    return std::string_view(reinterpret_cast<const char *>(this + 1), this->len);
}

#endif
