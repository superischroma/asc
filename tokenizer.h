#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <fstream>
#include <map>

namespace asc
{
    extern std::string TOKENIZER_REGEX_PATTERN;

    class syntax_node;

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
        unsigned long long options;
    public:
        tokenizer(std::ifstream& is);
        int lines();
        bool extractable();
        tokenizer& operator>>(std::string& token);
        syntax_node* syntax_start();
        std::string stringify_syntax();
        ~tokenizer();
    };

    syntax_node* tokenize(std::ifstream& is);
}

#endif