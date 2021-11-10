#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <type_traits>

#include "assembler.h"
#include "symbol.h"
#include "logger.h"

namespace asc
{
    namespace syntax_types
    {
        const unsigned short PROGRAM_BEGIN = 0;
        const unsigned short KEYWORD = 1;
        const unsigned short IDENTIFIER = 2;
        const unsigned short PUNCTUATOR = 3;
        const unsigned short CONSTANT = 4;
        const unsigned short STRING_LITERAL = 5;

        std::string name(unsigned short type);
    }

    unsigned char is_keyword(std::string& test);

    unsigned char is_punctuator(std::string& test);

    int get_operator(std::string& test);

    char get_visibility_id(std::string& test);

    bool is_numerical(std::string& test);

    bool is_primitive(std::string& test);

    int get_type_size(std::string& prim);

    std::string get_word(int size);

    std::string& unwrap(std::string& sl);

    class syntax_node
    {
    public:
        syntax_node* next;
        unsigned short type;
        std::string* value;
        int line;

        syntax_node(syntax_node* next, unsigned short type, std::string value, int line);
        std::string stringify();
        ~syntax_node();
    };
}

#endif