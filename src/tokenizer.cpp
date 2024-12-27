#include "tokenizer.hpp"
#include "fancyerror.hpp"
#include <string>
#include <list>
#include <stdexcept>

#define SET_GROUP_VECTOR(...) GROUP_VECTOR.clear(); GROUP_VECTOR.assign({__VA_ARGS__})

std::string getTokenName(TokenType type) {
    int index = static_cast<int>(type);
    auto it = std::next(TOKENTYPE_MAPPING.begin(), index);
    return *it; // Assumes the list size matches the enum count
}

Tokenizer::Tokenizer(const std::string& code) : data(code), it(data.begin()), line_count(1), line_position(1) {}

// Helper methods
bool Tokenizer::is_digit(char c)        { return std::isdigit(c) || c == '.'; }
bool Tokenizer::is_operator(char c)     { return OPERATORS.find(c) != std::string::npos; }
bool Tokenizer::is_whitespace(char c)   { return WHITESPACE.find(c) != std::string::npos; }
bool Tokenizer::is_paren(char c)        { return PAREN.find(c) != std::string::npos; }
bool Tokenizer::is_oneof(char c, const std::string& group) { return group.find(c) != std::string::npos; }
bool Tokenizer::is_word_start(char c)   { return is_oneof(c, WORD); }
bool Tokenizer::is_word(char c)         { return is_oneof(c, WORD) || is_oneof(c, NUMERIC); }
bool Tokenizer::is_keyword(const std::string &word) { return std::find(KEYWORDS.begin(), KEYWORDS.end(), word) != KEYWORDS.end(); }

bool Tokenizer::is_of(char c, const std::string &group){
    return group.find(c) != std::string::npos;
}

char Tokenizer::eat() {
    char v = *it++;
    line_position++;
    if (v == '\n') {
        line_count++;
        line_position = 1;
    }
    return v;
}

char Tokenizer::peek(int offset = 0) {
    if (it + offset >= data.end()) return '\0'; // Safeguard against going beyond end
    return *(it + offset);
}

std::string Tokenizer::consume(const std::vector<std::string> &groups) {
    std::string result;
    while (true){
        char c = peek();
        bool abort = true;
        for (const auto group : groups){
            if (is_of(c, group)){
                abort = false;
                break;
            }
        }
        if (abort || c == '\0'){
            break;
        }
        result += eat();
    }
    return result;
}

Token Tokenizer::peek_token(unsigned int offset){
    Token token;
    while(token_queue.size() <= offset){
        token = next_token(false);
        if (token.type == TokenType::EOF_TOKEN){
            return token;
        }
        token_queue.push_back(token);
    }
    return token_queue.at(offset);
}

Token Tokenizer::next_token(bool use_queue) {
    Token token{};
    if (use_queue && token_queue.size()>0){
        token = token_queue.front();
        token_queue.erase(token_queue.begin());
        return token;
    }

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
    else if (is_oneof(*it, STRING)){
        char opening = eat();
        while(true){
            if (it == data.end()) throw std::runtime_error("String not terminated!");
            char c = *it;
            if (c == opening){
                eat();
                break;
            }
            token.value.push_back(eat());
        }
        token.type = TokenType::STRING;
    }

    // Word or keyword
    else if (is_word_start(*it)) {
        SET_GROUP_VECTOR(WORD,NUMERIC);
        token.value = consume(GROUP_VECTOR);
        token.type = is_keyword(token.value) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
    }
    // Number
    else if (is_oneof(*it, NUMERIC)) {
        SET_GROUP_VECTOR(NUMERIC,NUMERIC_PUNCTUATION);
        token.value = consume(GROUP_VECTOR);
        token.type = TokenType::NUMBER;
    }
    // Operators
    else if (is_operator(*it)) {
        SET_GROUP_VECTOR(OPERATORS);
        token.value = consume(GROUP_VECTOR);
        token.type = TokenType::OPERATOR;
    }
    // Punctuation and parentheses
    else if (is_oneof(*it, PUNCTUATION) || is_paren(*it)) {
        token.value.push_back(eat());
        token.type = TokenType::PUNCTUATION;
    }
    else {
        std::string error = "Unknown character ";
        error.push_back(*it);
        std::cerr << MakePrettyError(data, line_count, line_position, error, 1, 4);
        throw std::runtime_error(error);
    }

    return token;
}

#undef GROUPS