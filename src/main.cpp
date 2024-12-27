#include <string>
#include <iostream>
#include "tokenizer.hpp"

int main() {
    std::string CODE = R"(
//this is a comment
let x = 10.5;
let name = "some name HERE 'with other' else";
let name2 = 'some name HERE "with other" else';
let y = 20; // another comment
let /*a = /*14*/;
let*/ b = 26;
mut(a) += x;
print(x+y);
)";

    Tokenizer tokenizer(CODE);
    while (true) {
        Token tok = tokenizer.next_token();
        if (tok.type == TokenType::EOF_TOKEN) break;
        std::cout << "line " << tok.line_number << ", position " << tok.line_position << ": " << getTokenName(tok.type) << ": " << tok.value 
                  << std::endl;
        // tok = tokenizer.peek_token(5);
        // std::cout << "line " << tok.line_number << ", position " << tok.line_position << ": " << getTokenName(tok.type) << ": " << tok.value 
        //           << std::endl;
    }
    return 0;
}