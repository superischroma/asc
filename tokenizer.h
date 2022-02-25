#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

namespace asc
{
    extern std::string TOKENIZER_REGEX_PATTERN;

    class syntax_node;

    syntax_node* tokenize(std::ifstream& is);
}

#endif