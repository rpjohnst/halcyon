#ifndef PARSER_HPP
#define PARSER_HPP
#include "tokenizer.hpp"
#include <string>

class Parser{
private:
    Tokenizer tokenstream;
public:
    Parser(std::string &code);
};

#endif