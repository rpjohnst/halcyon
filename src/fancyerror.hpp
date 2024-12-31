// #ifndef FANCYERROR_HPP
// #define FANCYERROR_HPP
// #include <iostream>
// #include <string>
// #include "tokenizer.hpp"
// #include <stdexcept>

// #define RED "\033[31m"
// #define RESET "\033[0m"

// std::string MakePrettyError(std::string &code, int line_number, int line_position, std::string message, int width = 1, int capture_lines = 4){
//     std::string output = "";
//     std::string lastlines = "";
//     std::string::iterator it = code.begin();
//     int last_padsize = 0;

//     int ln = 1;
//     int lp = 1;
//     while(it != code.end() && ln <= line_number){
//         if (ln > line_number-capture_lines){
//             if (lp == 1){
//                 std::string ps = std::to_string(ln)+"| ";
//                 last_padsize = ps.size()-1;
//                 lastlines += ps;
//             }
//             if (*it == '\n'){
//                 ln++;
//                 lp=1;
//                 lastlines.push_back(*it);
//                 it++;
//                 continue;
//             }
//             if (ln == line_number){
//                 if (lp == line_position) lastlines += RED;
//                 if (lp == line_position+width) lastlines += RESET;
//             }
//             lastlines.push_back(*it);
//         }
//         if (*it == '\n'){
//             ln++;
//             lp=0;
//         }
//         lp++;
//         it++;
//     }
//     output += lastlines;

//     lp = 1;
//     int DOWN = 1;
//     int RIGHT = 3;
//     std::string strpad = "";
//     while (lp <= line_position+last_padsize){
//         strpad += " ";
//         lp++;
//     }
    
//     output += strpad;
//     for(auto i = 0; i < width; i++){
//         output += "^";
//     }
//     output += "\n";
//     output += strpad+"\033[31m"+message+"\033[0m\n";
//     output += strpad+"\033[31mline: "+std::to_string(line_number)+" pos: "+std::to_string(line_position)+"\033[0m";
//     return output;
// }

// #endif
#ifndef FANCYERROR_HPP
#define FANCYERROR_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "tokenizer.hpp"

#define RED "\033[31m"
#define RESET "\033[0m"

// Function to format an error message with source code context
std::string MakePrettyError(const std::string& code, int line_number, int line_position, const std::string& message, int width = 1, int capture_lines = 4) {
    std::ostringstream output;
    std::istringstream iss(code);
    std::vector<std::string> lines;
    std::string line;

    // Split code into lines
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    // Ensure we don't go out of bounds
    if (line_number < 1 || line_number > static_cast<int>(lines.size())) {
        throw std::out_of_range("Line number out of range");
    }

    int start_line = std::max(1, line_number - capture_lines + 1);
    int end_line = std::min(static_cast<int>(lines.size()), line_number);

    // Print lines before, at, and after the error
    for (int i = start_line; i <= end_line; ++i) {
        std::string prefix = std::to_string(i) + "| ";
        output << prefix;
        if (i == line_number) {
            // Highlight error position
            for (size_t j = 0; j < lines[i-1].length(); ++j) {
                if (j == static_cast<size_t>(line_position - 1)) {
                    output << RED;
                }
                if (j == static_cast<size_t>(line_position - 1 + width)) {
                    output << RESET;
                }
                output << lines[i-1][j];
            }
            if (lines[i-1].length() < static_cast<size_t>(line_position - 1)) {
                // If position is beyond the line length, just print the line
                output << RESET;
            }
        } else {
            output << lines[i-1];
        }
        output << '\n';
    }

    // Print error indicator
    std::string padding(std::to_string(line_number).length() + 2 + line_position - 1, ' ');
    output << padding;
    output << std::string(width, '^') << '\n'
           << padding << RED << message << RESET << '\n'
           << padding << RED << "line: " << line_number << " pos: " << line_position << RESET;

    return output.str();
}

#endif