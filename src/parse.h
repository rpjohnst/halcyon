#ifndef PARSE_H
#define PARSE_H

#include "symbol.h"

#include <vector>
#include <string_view>
#include <cstdint>

class Arena;
enum class TreeKind : uint8_t;
struct Tree;

class Parse {
    Arena &arena;
    Symbol::Intern &intern;

    // This stack holds partial sequences of children for in-progress tree nodes,
    // before the current lookahead token. It is split in two segments, with any
    // topmost sequence of trivia (whitespace and comments) stored in `space`.
    // When a token is consumed, both the trivia and token are pushed onto `stack`.
    std::vector<Tree *> stack;
    std::vector<Tree *> space;

    // The current lookahead token, and its position in the input buffer.
    TreeKind token;
    const char *text;

    // The remaining input after the lookahead token. `end` must point to a nul byte
    // for the lexer to use as a sentinel, eliding most comparisons with `end`.
    const char *begin;
    const char *end;

public:
    Parse(Arena &arena, Symbol::Intern &intern, std::string_view source);

    static auto name(TreeKind kind) -> std::string_view;

    auto module() -> Tree *;

private:
    auto expression(TreeKind parent) -> bool;
    auto constructor() -> void;
    auto object() -> void;

    auto pattern() -> bool;
    auto destructor() -> void;

    auto arrow() -> void;

    auto expect(TreeKind expected) -> void;
    auto expect(std::string_view expected) -> void;
    auto eat(TreeKind expected) -> bool;
    auto shift() -> void;
    auto read() -> void;

    auto mark() -> size_t;
    auto reduce(TreeKind kind, size_t start) -> Tree *;
};

#endif
