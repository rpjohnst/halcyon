#ifndef FANCYERROR_HPP
#define FANCYERROR_HPP
#include <iostream>
#include <string>
#include "tokenizer.hpp"
#include <stdexcept>

#define RED "\033[31m"
#define RESET "\033[0m"

std::string MakePrettyError(std::string &code, int line_number, int line_position, std::string message, int width = 1, int capture_lines = 4){
    std::string output = "";
    std::string lastlines = "";
    std::string::iterator it = code.begin();
    int last_padsize = 0;

    int ln = 1;
    int lp = 1;
    while(it != code.end() && ln <= line_number){
        if (ln > line_number-capture_lines){
            if (lp == 1){
                std::string ps = std::to_string(ln)+"| ";
                last_padsize = ps.size()-1;
                lastlines += ps;
            }
            if (*it == '\n'){
                ln++;
                lp=1;
                lastlines.push_back(*it);
                it++;
                continue;
            }
            if (ln == line_number){
                if (lp == line_position) lastlines += RED;
                if (lp == line_position+width) lastlines += RESET;
            }
            lastlines.push_back(*it);
        }
        if (*it == '\n'){
            ln++;
            lp=0;
        }
        lp++;
        it++;
    }
    output += lastlines;

    lp = 1;
    int DOWN = 1;
    int RIGHT = 3;
    std::string strpad = "";
    while (lp <= line_position+last_padsize){
        strpad += " ";
        lp++;
    }
    
    output += strpad;
    for(auto i = 0; i < width; i++){
        output += "^";
    }
    output += "\n";
    output += strpad+"\033[31m"+message+"\033[0m\n";
    output += strpad+"\033[31mline: "+std::to_string(line_number)+" pos: "+std::to_string(line_position)+"\033[0m";
    return output;
}

#endif