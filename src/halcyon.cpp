#include "parse.h"
#include "syntax.h"
#include "symbol.h"
#include "arena.h"

#include <algorithm>
#include <print>

namespace {
auto print(uint32_t indent, Tree *tree) -> void;
}

auto main() -> int {
    using namespace std::literals;

    constexpr auto source =
        "fibonacci(n) = n {\n"
        "    0 -> 0,\n"
        "    1 -> 1,\n"
        "    _ -> fibonacci(n - 1) + fibonacci(n - 2),\n"
        "}\n\0"sv;

    auto arena = Arena{};
    auto intern = Symbol::Intern{};
    auto parse = Parse{arena, intern, source};
    auto module = parse.module();

    print(0, module);
    std::println("");
}

namespace {
auto print(uint32_t indent, Tree *tree) -> void {
    using namespace std::literals;

    std::print("{:{}}", "", 2 * indent);

    auto children = tree->children();
    if (children.size() > 0) { std::print("("); }

    if (tree->kind < TreeKind::Module) { std::print("\""); }
    std::print("{}", Parse::name(tree->kind));
    if (tree->kind < TreeKind::Module) { std::print("\""); }

    if (std::any_of(children.begin(), children.end(), [](Tree *child) { return child->arity; })) {
        for (auto child : children) { std::println(""); print(indent + 1, child); }
    } else {
        for (auto child : children) { std::print(" "); print(0, child); }
    }

    if (children.size() > 0) { std::print(")"); }
}
}
