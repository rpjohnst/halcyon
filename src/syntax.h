#ifndef SYNTAX_H
#define SYNTAX_H

#include <span>
#include <cstdint>

enum class TreeKind : uint8_t {
    Error, End, Space, Comment,

    Name, Number, String,
    Plus, Minus, Star, Slash, Less, Equal, Greater, Dot, Comma, Semi,
    LeftParen, RightParen, LeftBracket, RightBracket, LeftBrace, RightBrace,

    Module, Definition,
    ParenExpr, ConsExpr, CaseExpr, CutExpr, BinaryExpr,
    Method, Arrow,
};

struct Tree {
    TreeKind kind;
    uint32_t width;
    uint32_t arity;

    Tree(TreeKind kind, uint32_t width, std::span<Tree *> children);

    std::span<Tree *> children();
};

inline std::span<Tree *> Tree::children() {
    return std::span(reinterpret_cast<Tree **>(this + 1), this->arity);
}

#endif
