#include "syntax.h"

#include <algorithm>

Tree::Tree(TreeKind kind, uint32_t width, std::span<Tree *> children) :
    kind(kind), width(width), arity(children.size())
{
    std::copy(children.begin(), children.end(), reinterpret_cast<Tree **>(this + 1));
}
