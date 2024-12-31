#include "parse.h"
#include "syntax.h"
#include "arena.h"

#include <iterator>
#include <print>
#include <span>
#include <cassert>

using namespace std::literals;

Parse::Parse(Arena &arena, Symbol::Intern &intern, std::string_view source) :
    arena(arena), intern(intern), begin(source.begin()), end(source.end() - 1)
{
    assert(*end == '\0');
    this->read();
}

auto Parse::name(TreeKind kind) -> std::string_view {
    constexpr auto names = std::array{
        "error"sv, "end of file"sv, "whitespace"sv, "comment"sv,
        "name"sv, "number"sv, "string"sv,
        "+"sv, "-"sv, "*"sv, "/"sv, "<"sv, "="sv, ">"sv, "."sv, ","sv, ";"sv,
        "("sv, ")"sv, "["sv, "]"sv, "{"sv, "}"sv,

        "module"sv, "definition"sv,
        "parentheses"sv, "constructor"sv, "case"sv, "cut"sv, "binary"sv,

        "method"sv, "->"sv,
    };
    return names[size_t(kind)];
}

auto Parse::module() -> Tree * {
    while (this->token != TreeKind::End) {
        auto start = this->mark();
        if (this->eat(TreeKind::Name)) {
            if (this->token != TreeKind::Equal) {
                this->pattern();
                this->reduce(TreeKind::CutExpr, start);
            }
            this->expect(TreeKind::Equal);
            this->expression(TreeKind::Definition);
            this->reduce(TreeKind::Definition, start);
        } else {
            this->expect("definition"sv);
            this->shift();
            this->reduce(TreeKind::Error, start);
        }
    }
    return this->reduce(TreeKind::Module, 0);
}

namespace {
auto right_child(TreeKind parent, TreeKind child) -> bool;
}

auto Parse::expression(TreeKind parent) -> bool {
    auto start = this->mark();
    if (this->token == TreeKind::Name) {
        auto text = std::string_view(this->text, this->begin - this->text);
        auto symbol = Symbol::intern(this->arena, this->intern, text);
        this->constructor();
    } else if (this->eat(TreeKind::Number)) {
    } else if (this->eat(TreeKind::String)) {
    } else if (this->token == TreeKind::LeftBrace) {
        this->object();
    } else if (this->eat(TreeKind::LeftParen)) {
        this->expression(TreeKind::ParenExpr);
        this->expect(TreeKind::RightParen);
        this->reduce(TreeKind::ParenExpr, start);
    } else {
        this->expect("expression"sv);
        return false;
    }

    while (true) {
        if (this->token == TreeKind::Name) {
            auto text = std::string_view(this->text, this->begin - this->text);
            auto symbol = Symbol::intern(this->arena, this->intern, text);
            this->constructor();
        } else if (this->token == TreeKind::LeftParen) {
            this->constructor();
        } else if (this->token == TreeKind::LeftBrace) {
            this->object();
        } else {
            break;
        }
        this->reduce(TreeKind::CutExpr, start);
    }

    while (right_child(parent, this->token)) {
        this->shift();
        this->expression(this->token);
        this->reduce(TreeKind::BinaryExpr, start);
    }

    return true;
}

namespace {
auto right_child(TreeKind parent, TreeKind child) -> bool {
    switch (child) {
    case TreeKind::Plus: case TreeKind::Minus:
        switch (parent) {
        case TreeKind::Plus: case TreeKind::Minus: return false;
        case TreeKind::Star: case TreeKind::Slash: return false;
        default: return true;
        }

    case TreeKind::Star: case TreeKind::Slash:
        switch (parent) {
        case TreeKind::Plus: case TreeKind::Minus: return true;
        case TreeKind::Star: case TreeKind::Slash: return false;
        default: return true;
        }

    default: return false;
    }
}
}

auto Parse::constructor() -> void {
    auto start = this->mark();
    this->eat(TreeKind::Name);
    if (this->eat(TreeKind::LeftParen)) {
        while (this->token != TreeKind::End && this->token != TreeKind::RightParen) {
            if (!this->expression(TreeKind::ConsExpr)) { break; }
            if (this->token != TreeKind::RightParen) { this->expect(TreeKind::Comma); }
        }
        this->expect(TreeKind::RightParen);
        this->reduce(TreeKind::ConsExpr, start);
    }
}

auto Parse::object() -> void {
    auto start = this->mark();
    this->expect(TreeKind::LeftBrace);
    while (this->token != TreeKind::End && this->token != TreeKind::RightBrace) {
        auto start = this->mark();
        if (!this->pattern()) { break; }
        this->arrow();
        this->expression(TreeKind::Method);
        this->reduce(TreeKind::Method, start);
        if (this->token != TreeKind::RightBrace) { this->expect(TreeKind::Comma); }
    }
    this->expect(TreeKind::RightBrace);
    this->reduce(TreeKind::CaseExpr, start);
}

auto Parse::pattern() -> bool {
    auto start = this->mark();
    if (this->token == TreeKind::Name) {
        auto text = std::string_view(this->text, this->begin - this->text);
        auto symbol = Symbol::intern(this->arena, this->intern, text);
        this->destructor();
    } else if (this->token == TreeKind::LeftParen) {
        this->destructor();
    } else if (this->eat(TreeKind::Number)) {
    } else if (this->eat(TreeKind::String)) {
    } else {
        this->expect("pattern"sv);
        return false;
    }

    while (true) {
        auto start = this->mark();
        if (this->token == TreeKind::Name) {
            auto text = std::string_view(this->text, this->begin - this->text);
            auto symbol = Symbol::intern(this->arena, this->intern, text);
            this->destructor();
        } else if (this->token == TreeKind::LeftParen) {
            this->destructor();
        } else {
            break;
        }
        this->reduce(TreeKind::CutExpr, start);
    }

    return true;
}

auto Parse::destructor() -> void {
    auto start = this->mark();
    this->eat(TreeKind::Name);
    if (this->eat(TreeKind::LeftParen)) {
        while (this->token != TreeKind::End && this->token != TreeKind::RightParen) {
            if (!this->pattern()) { break; }
            if (this->token != TreeKind::RightParen) { this->expect(TreeKind::Comma); }
        }
        this->expect(TreeKind::RightParen);
        this->reduce(TreeKind::ConsExpr, start);
    }
}

namespace {
auto read_token(const char *&begin, const char *end) -> TreeKind;
}

auto Parse::arrow() -> void {
    if (this->token == TreeKind::Minus) {
        auto begin = this->begin;
        auto token = read_token(begin, this->end);
        if (token == TreeKind::Greater) {
            this->begin = begin;
            this->token = TreeKind::Arrow;
        }
    }

    this->expect(TreeKind::Arrow);
}

auto Parse::expect(TreeKind expected) -> void {
    if (!this->eat(expected)) { this->expect(Parse::name(expected)); }
}

auto Parse::expect(std::string_view expected) -> void {
    auto found = std::string_view(this->text, this->begin - this->text);
    std::println(stderr, "error: expected {}; found {}", expected, Parse::name(this->token));
}

auto Parse::eat(TreeKind expected) -> bool {
    if (this->token == expected) {
        this->shift();
        return true;
    } else {
        return false;
    }
}

namespace {
constexpr auto empty = std::span<Tree *>{};
}

auto Parse::shift() -> void {
    this->stack.insert(
        this->stack.end(),
        std::make_move_iterator(this->space.begin()), std::make_move_iterator(this->space.end()));
    this->space.clear();

    auto tree = arena.alloc<Tree>(this->token, uint32_t(this->begin - this->text), empty);
    this->stack.push_back(tree);

    this->read();
}

auto Parse::read() -> void {
    while (true) {
        this->text = this->begin;
        this->token = read_token(this->begin, this->end);

        if (this->token != TreeKind::Space && this->token != TreeKind::Comment) { break; }

        auto tree = arena.alloc<Tree>(this->token, uint32_t(this->begin - this->text), empty);
        this->space.push_back(tree);
    }
}

auto Parse::mark() -> size_t { return this->stack.size() + this->space.size(); }

auto Parse::reduce(TreeKind kind, size_t start) -> Tree * {
    auto children = std::span(this->stack.begin() + start, this->stack.end());

    uint32_t width = 0;
    for (auto child : children) { width += child->width; }

    auto p = arena.alloc(sizeof(Tree) + sizeof(Tree *) * children.size(), alignof(Tree));
    auto tree = new (p) Tree(kind, width, children);

    this->stack.resize(start);
    this->stack.push_back(tree);
    return tree;
}

namespace {
auto make_token(const char *&begin, const char *end, TreeKind kind) -> TreeKind;

auto read_token(const char *&begin, const char *end) -> TreeKind {
    const char *p = begin;
    char c = *p++;

    // \0$ - Discard p so subsequent calls return End again.
    if (c == '\0' && begin == end) { return TreeKind::End; }

    // [\t\n\r ]+
    if (c == '\t' || c == '\n' || c == '\r' || c == ' ') {
        while (*p == '\t' || *p == '\n' || *p == '\r' || *p == ' ') { p++; }
        return make_token(begin, p, TreeKind::Space);
    }

    // "([^"\n]|\\")*"
    if (c == '"') {
        while (*p != '\0' && *p != '\n' && *p != '"') {
            if (p[0] == '\\' && p[1] == '"') { p++; }
            p++;
        }
        if (*p == '"') { p++; }
        return make_token(begin, p, TreeKind::String);
    }

    // #.*\n
    if (c == '#') {
        while (*p != '\0' && *p != '\n') { p++; }
        if (*p == '\n') { p++; }
        return make_token(begin, p, TreeKind::Comment);
    }

    if (c == '(') { return make_token(begin, p, TreeKind::LeftParen); }
    if (c == ')') { return make_token(begin, p, TreeKind::RightParen); }
    if (c == '*') { return make_token(begin, p, TreeKind::Star); }
    if (c == '+') { return make_token(begin, p, TreeKind::Plus); }
    if (c == ',') { return make_token(begin, p, TreeKind::Comma); }
    if (c == '-') { return make_token(begin, p, TreeKind::Minus); }
    if (c == '.') { return make_token(begin, p, TreeKind::Dot); }
    if (c == '/') { return make_token(begin, p, TreeKind::Slash); }

    // [0-9']+(\.[0-9']+)?
    if ('0' <= c && c <= '9') {
        while (true) {
            if ('0' <= *p && *p <= '9') {}
            else if (*p == '\'') {}
            else { break; }
            p++;
        }
        if (p[0] == '.' && '0' <= p[1] && p[1] <= '9') {
            p += 2;
            while (true) {
                if ('0' <= *p && *p <= '9') {}
                else if (*p == '\'') {}
                else { break; }
                p++;
            }
        }
        return make_token(begin, p, TreeKind::Number);
    }

    if (c == ';') { return make_token(begin, p, TreeKind::Semi); }
    if (c == '<') { return make_token(begin, p, TreeKind::Less); }
    if (c == '=') { return make_token(begin, p, TreeKind::Equal); }
    if (c == '>') { return make_token(begin, p, TreeKind::Greater); }

    // [A-Za-z_][A-Za-z'_-]*
    if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_') {
        while (true) {
            if (*p == '\'' || *p == '-' || *p == '_') {}
            else if ('0' <= *p && *p <= '9') {}
            else if ('A' <= *p && *p <= 'Z') {}
            else if ('a' <= *p && *p <= 'z') {}
            else { break; }
            p++;
        }
        return make_token(begin, p, TreeKind::Name);
    }

    if (c == '[') { return make_token(begin, p, TreeKind::LeftBracket); }
    if (c == ']') { return make_token(begin, p, TreeKind::RightBracket); }
    if (c == '{') { return make_token(begin, p, TreeKind::LeftBrace); }
    if (c == '}') { return make_token(begin, p, TreeKind::RightBrace); }

    return make_token(begin, p, TreeKind::Error);
}

auto make_token(const char *&begin, const char *end, TreeKind kind) -> TreeKind {
    begin = end;
    return kind;
}
}
