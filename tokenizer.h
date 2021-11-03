#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <fstream>
#include <map>
#include "syntax.h"

namespace asc
{
    class tokenizer
    {
    private:
        std::ifstream* is;
        int linet;
        std::string later;
        asc::syntax_node* syntax_head;
        asc::syntax_node* current_node;
        asc::syntax_node* last_identifier;
        asc::syntax_node* scope;
        unsigned char comment; // 0 - no comment, 1 - one line comment, 2 - block comment
    public:
        tokenizer(std::ifstream& is);
        int lines();
        bool extractable();
        tokenizer& operator>>(std::string& token);
        syntax_node* syntax_start();
        std::string stringify_syntax();
        ~tokenizer();
    };
}

#endif