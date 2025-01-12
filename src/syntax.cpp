#include "syntax.h"

#include <memory>
#include <array>

Tree::Tree(TreeKind kind, bool visible, uint32_t width, std::span<Tree *> children) :
    kind(kind), visible(visible), width(width), arity(children.size())
{
    std::uninitialized_copy(children.begin(), children.end(), this->children().begin());
}

auto Tree::name(TreeKind kind) -> std::string_view {
    using namespace std::literals;

    constexpr auto names = std::array{
        "error"sv, "end of file"sv, "whitespace"sv, "comment"sv,
        "name"sv, "number"sv, "string"sv,
        "+"sv, "-"sv, "*"sv, "/"sv, "<"sv, "="sv, ">"sv, "."sv, ","sv, ";"sv,
        "("sv, ")"sv, "["sv, "]"sv, "{"sv, "}"sv,

        "binding"sv, "constructor"sv, "object"sv, "cut"sv, "parentheses"sv, "binary"sv,
        "list"sv, "->"sv,
    };
    return names[size_t(kind)];
}
