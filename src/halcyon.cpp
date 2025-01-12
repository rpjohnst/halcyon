#include "parse.h"
#include "syntax.h"
#include "symbol.h"
#include "arena.h"

#include <algorithm>
#include <print>

namespace {
auto print(uint32_t indent, Syntax node) -> void;
}

auto main() -> int {
    using namespace std::literals;

    constexpr auto source =
        "fibonacci(n) = n {\n"
        "    0 -> 0,\n"
        "    1 -> 1,\n"
        "    _ -> fibonacci(n - 1) + fibonacci(n - 2),\n"
        "};\n"
        "\n"
        "factorial(n) = n {\n"
        "    1 -> 1,\n"
        "    _ -> n * factorial(n - 1),\n"
        "}\0"sv;

    auto arena = Arena{};
    auto intern = Symbol::Intern{};
    auto definitions = module(arena, intern, source);

    print(0, definitions.node);
    std::println("");
}

namespace {
auto print(uint32_t indent, Syntax node) -> void {
    using namespace std::literals;

    std::print("{:{}}", "", 2 * indent);

    if (node.arity() > 0) { std::print("("); }

    if (node.kind() < TOKENS) {
        std::print("{:?}", node.source());
    } else {
        std::print("{}", Tree::name(node.kind()));
    }

    if (std::any_of(node.begin(), node.end(), [](Syntax child) { return child.arity(); })) {
        for (auto child : node) { std::println(""); print(indent + 1, child); }
    } else {
        for (auto child : node) { std::print(" "); print(0, child); }
    }

    if (node.arity() > 0) { std::print(")"); }
}
}
