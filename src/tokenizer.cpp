#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
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

std::string getTokenName(TokenType type) {
    int index = static_cast<int>(type);
    auto it = std::next(TOKENTYPE_MAPPING.begin(), index);
    return *it; // Assumes the list size matches the enum count
}

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
    bool is_digit(char c) { return std::isdigit(c) || c == '.'; }
    bool is_operator(char c) { return OPERATORS.find(c) != std::string::npos; }
    bool is_whitespace(char c) { return WHITESPACE.find(c) != std::string::npos; }
    bool is_paren(char c) { return PAREN.find(c) != std::string::npos; }
    bool is_oneof(char c, const std::string& group) { return group.find(c) != std::string::npos; }
    bool is_word_start(char c) { return is_oneof(c, WORD); }
    bool is_word(char c) { return is_oneof(c, WORD) || is_oneof(c, NUMERIC); }
    bool is_keyword(const std::string &word) { return std::find(KEYWORDS.begin(), KEYWORDS.end(), word) != KEYWORDS.end(); }

    char eat() {
        char v = *it++;
        line_position++;
        if (v == '\n') {
            line_count++;
            line_position = 1;
        }
        return v;
    }

    char peek(int offset = 0) const {
        if (it + offset >= data.end()) return '\0'; // Safeguard against going beyond end
        return *(it + offset);
    }

    std::string consume_word() {
        std::string result;
        while (it != data.end() && is_word(*it)) {
            result.push_back(eat());
        }
        return result;
    }

    std::string consume_number() {
        std::string result;
        while (it != data.end() && (is_oneof(*it, NUMERIC) || *it == '.')) {
            result.push_back(eat());
        }
        return result;
    }

public:
    Tokenizer(const std::string& code) : data(code), it(data.begin()), line_count(1), line_position(1) {}

    Token next_token() {
        Token token{};

        while (it != data.end() && is_whitespace(*it)) {
            eat(); // Consume whitespace
        }

        token.line_number = line_count;
        token.line_position = line_position;

        if (it == data.end()) {
            token.type = TokenType::EOF_TOKEN;
            return token;
        }

        // Single-line comment
        if (*it == '/' && peek(1) == '/') {
            while (it != data.end() && *it != '\n') {
                token.value.push_back(eat());
            }
            if (it != data.end()) eat(); // Consume the newline at the end of a comment
            token.type = TokenType::COMMENT;
        }
        // Multi-line comment
        else if (*it == '/' && peek(1) == '*') {
            int depth = 1;
            token.value.push_back(eat()); // Consume '/' to prevent another depth match
            while (depth > 0) {
                if (it == data.end()) throw std::runtime_error("Multiline comment not terminated!");
                if (*it == '/' && peek(1) == '*') depth++;
                else if (*it == '*' && peek(1) == '/') depth--;
                token.value.push_back(eat());
            }
            token.value.push_back(eat()); // Consume the last '/'
            token.type = TokenType::COMMENT;
        }
        // Word or keyword
        else if (is_word_start(*it)) {
            token.value = consume_word();
            token.type = is_keyword(token.value) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
        }
        // Number
        else if (is_oneof(*it, NUMERIC)) {
            token.value = consume_number();
            token.type = TokenType::NUMBER;
        }
        // Operators
        else if (is_operator(*it)) {
            while (it != data.end() && is_operator(*it)) {
                token.value.push_back(eat());
            }
            token.type = TokenType::OPERATOR;
        }
        // Punctuation and parentheses
        else if (is_oneof(*it, PUNCTUATION) || is_paren(*it)) {
            token.value.push_back(eat());
            token.type = TokenType::PUNCTUATION;
        }
        else {
            std::string error = "Tokenizer: Unknown character ";
            error.push_back(*it);
            throw std::runtime_error(error);
        }

        return token;
    }
};