#ifndef SYNTAX_H
#define SYNTAX_H

#include <string_view>
#include <span>
#include <cassert>
#include <cstdint>

enum class TreeKind : uint8_t {
    Error, End, Space, Comment,

    Name, Number, String,
    Plus, Minus, Star, Slash, Less, Equal, Greater, Dot, Comma, Semi,
    LeftParen, RightParen, LeftBracket, RightBracket, LeftBrace, RightBrace,

    BindExpr, ConsExpr, CaseExpr, CutExpr, ParenExpr, BinaryExpr,
    DelimList, Arrow,
};
constexpr auto TOKENS = TreeKind::BindExpr;

struct alignas(struct Tree *) Tree {
    TreeKind kind;
    bool visible;
    uint32_t width;
    uint32_t arity;

    Tree(TreeKind kind, bool visible, uint32_t width, std::span<Tree *> children);

    auto children() -> std::span<Tree *>;

    static auto name(TreeKind kind) -> std::string_view;
};

inline auto Tree::children() -> std::span<Tree *> {
    return std::span(reinterpret_cast<Tree **>(this + 1), this->arity);
}

template <class R>
struct ArrowProxy {
    R r;
    auto operator->() -> R * { return &r; }
};

struct Syntax {
    Tree *tree;
    const char *text;

    auto kind() -> TreeKind { return tree->kind; }
    auto visible() -> bool { return tree->visible; }
    auto arity() -> uint32_t { return tree->arity; }

    auto source() -> std::string_view { return std::string_view(this->text, this->tree->width); }

    struct Iterator {
        Tree **tree;
        const char *text;

        auto operator*() const -> Syntax { return Syntax{*this->tree, this->text}; }
        auto operator->() const -> ArrowProxy<Syntax> { return ArrowProxy{**this}; }

        auto operator++() -> Iterator & {
            auto tree = *this->tree++;
            this->text += tree->width;
            return *this;
        }
        auto operator++(int) -> Iterator { auto it = *this; ++*this; return it; }

        friend auto operator==(const Iterator &a, const Iterator &b) -> bool {
            return a.tree == b.tree;
        }
    };
    auto begin() -> Iterator {
        return Iterator{&*this->tree->children().begin(), this->text};
    }
    auto end() -> Iterator {
        return Iterator{&*this->tree->children().end(), this->text + this->tree->width};
    }
};

struct Visible : Syntax {
    Visible(Syntax node) : Syntax(node) { assert(node.visible()); }

    struct Iterator {
        Visible &parent;
        Syntax::Iterator current;

        auto operator*() const -> Visible { return Visible{*current}; }
        auto operator->() const -> ArrowProxy<Visible> { return ArrowProxy{**this}; }

        auto operator++() -> Iterator & {
            this->current = Visible::find(++this->current, this->parent.Syntax::end());
            return *this;
        }
        auto operator++(int) -> Iterator { auto it = *this; ++*this; return it; }

        friend auto operator==(const Iterator &a, const Iterator &b) -> bool {
            return a.current == b.current;
        }
    };
    auto begin() -> Iterator {
        return Iterator{*this, find(this->Syntax::begin(), this->Syntax::end())};
    }
    auto end() -> Iterator { return Iterator{*this, this->Syntax::end()}; }

    static auto find(Syntax::Iterator it, const Syntax::Iterator &end) -> Syntax::Iterator {
        while (it != end && !it->visible()) { ++it; }
        return it;
    }
};

struct BindExpr {
    Visible pattern;
    Visible expression;

    static auto from(Visible node) -> BindExpr {
        assert(node.kind() == TreeKind::BindExpr);

        auto it = node.begin();
        auto pattern = *it++;
        it++;
        auto expression = *it++;

        return BindExpr{pattern, expression};
    }
};

struct ConsExpr {
    Visible variant;
    Visible arguments;

    static auto from(Visible node) -> ConsExpr {
        assert(node.kind() == TreeKind::ConsExpr);

        auto it = node.begin();
        auto variant = *it++;
        it++;
        auto arguments = *it++;
        it++;

        return ConsExpr{variant, arguments};
    }
};

struct CaseExpr {
    Visible variants;

    static auto from(Visible node) -> CaseExpr {
        assert(node.kind() == TreeKind::CaseExpr);

        auto it = node.begin();
        it++;
        auto variants = *it++;
        it++;

        return CaseExpr{variants};
    }
};

struct CutExpr {
    Visible a;
    Visible b;

    static auto from(Visible node) -> CutExpr {
        assert(node.kind() == TreeKind::CutExpr);

        auto it = node.begin();
        auto a = *it++;
        auto b = *it++;

        return CutExpr{a, b};
    }
};

struct ParenExpr {
    Visible expression;

    static auto from(Visible node) -> ParenExpr {
        assert(node.kind() == TreeKind::ParenExpr);

        auto it = node.begin();
        it++;
        auto expression = *it++;
        it++;

        return ParenExpr{expression};
    }
};

struct BinaryExpr {
    Visible a;
    Visible op;
    Visible b;

    static auto from(Visible node) -> BinaryExpr {
        assert(node.kind() == TreeKind::BinaryExpr);

        auto it = node.begin();
        auto a = *it++;
        auto op = *it++;
        auto b = *it++;

        return BinaryExpr{a, op, b};
    }
};

struct DelimList {
    Visible node;

    static auto from(Visible node) -> DelimList {
        assert(node.kind() == TreeKind::DelimList);
        return DelimList{node};
    }

    struct Iterator {
        Visible::Iterator current;

        auto operator*() const -> Visible { return *current; }
        auto operator->() const -> Visible::Iterator { return current; }

        auto operator++() -> Iterator & {
            if (++this->current != this->current.parent.end()) { ++this->current; }
            return *this;
        }
        auto operator++(int) -> Iterator { auto it = *this; ++*this; return it; }

        friend auto operator==(const Iterator &a, const Iterator &b) -> bool {
            return a.current == b.current;
        }
    };
    auto begin() -> Iterator { return Iterator{this->node.begin()}; }
    auto end() -> Iterator { return Iterator{this->node.end()}; }
};

#endif
