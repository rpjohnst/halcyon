#ifndef PARSER_HPP
#define PARSER_HPP
#include "tokenizer.hpp"
#include <string>
#include <list>

class Parser{
private:
    Tokenizer tokenstream;
public:
    Parser(std::string &code);
    void ops();

};

#endif