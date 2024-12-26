#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP
#include <string>
#include <list>


const std::string OPERATORS = "+-*/^%=<>!";
const std::string WHITESPACE = " \r\n\t";
const std::string PUNCTUATION = ",.;";
const std::string PAREN = "()";
const std::string BRACES = "{}";
const std::string WORD = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
const std::string NUMERIC = "0123456789";
const std::list<std::string> KEYWORDS = {
    "let","mut","copy",
    "if", "else","func"
};

// Alphabetized enum
enum class TokenType {
    COMMENT,
    EOF_TOKEN,
    EQUALS,
    IDENTIFIER,
    KEYWORD,
    NONE,
    NUMBER,
    OPERATOR,
    PAREN,
    PUNCTUATION,
    STRING
};

// Alphabetized list matching the enum order
const std::list<std::string> TOKENTYPE_MAPPING = {
    "COMMENT",
    "EOF_TOKEN",
    "EQUALS",
    "IDENTIFIER",
    "KEYWORD",
    "NONE",
    "NUMBER",
    "OPERATOR",
    "PAREN",
    "PUNCTUATION",
    "STRING"
};

std::string getTokenName(TokenType type);

struct Token{
    TokenType type;
    std::string value;
    unsigned int line_number;
    unsigned int line_position;
};

class Tokenizer {
private:
    std::string data;
    std::string::iterator it;
    unsigned int line_count;
    unsigned int line_position;

    // Helper methods
    bool is_digit(char c);
    bool is_operator(char c);
    bool is_whitespace(char c);
    bool is_paren(char c);
    bool is_oneof(char c, const std::string& group);
    bool is_word_start(char c);
    bool is_word(char c);
    bool is_keyword(const std::string &word);
    char eat();
    char peek(int offset);
    std::string consume_word();
    std::string consume_number();

public:
    Tokenizer(const std::string& code);
    Token next_token();
};


#endif