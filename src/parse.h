#ifndef PARSE_H
#define PARSE_H

#include "symbol.h"

#include <string_view>

struct Arena;
struct DelimList;

auto module(Arena &arena, Symbol::Intern &intern, std::string_view text) -> DelimList;

#endif
