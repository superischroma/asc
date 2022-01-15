#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <type_traits>
#include <regex>

namespace asc // Forward declarations
{
    class syntax_node;
    typedef unsigned short visibility;
    typedef unsigned short primitive;
}

#include "assembler.h"
#include "symbol.h"
#include "logger.h"
#include "util.h"

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

    namespace visibilities
    {
        const unsigned short INVALID = -1;
        const unsigned short PUBLIC = 0;
        const unsigned short PRIVATE = 1;
        const unsigned short PROTECTED = 2;
        const unsigned short LOCAL = 3;

        std::string name(unsigned short type);
        visibility value_of(std::string name);
    }

    namespace primitives
    {
        const unsigned short PRIMITIVE_VOID = 0;
        const unsigned short PRIMITIVE_BYTE = 1;
        const unsigned short PRIMITIVE_SHORT = 2;
        const unsigned short PRIMITIVE_INT = 3;
        const unsigned short PRIMITIVE_LONG = 4;
        const unsigned short PRIMITIVE_BOOL = 5;
        const unsigned short PRIMITIVE_CHAR = 6;
        const unsigned short PRIMITIVE_FLOAT = 7;
        const unsigned short PRIMITIVE_DOUBLE = 8;

        std::string name(unsigned short type);
        primitive from_display(std::string name);
    }

    unsigned char is_keyword(std::string& test);

    unsigned char is_punctuator(std::string& test);

    int get_operator(std::string& test);

    char get_visibility_id(std::string& test);

    bool is_numerical(std::string& test);

    bool is_primitive(std::string& test);

    int get_type_size(std::string& prim);

    int get_register_size(std::string& reg);

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
        bool operator==(std::string value);
        bool operator!=(std::string value);
        ~syntax_node();
    };
}

#endif