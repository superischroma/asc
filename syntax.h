#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <type_traits>
#include <regex>
#include <map>

#define LEFT_OPERATOR_ASSOCATION true
#define RIGHT_OPERATOR_ASSOCATION false

#define PREFIX_OPERATOR 0
#define INFIX_OPERATOR 1
#define SUFFIX_OPERATOR 2

namespace asc // Forward declarations
{
    class syntax_node;
}

#include "symbol.h" // christ almighty

namespace asc
{
    class function_symbol; // forward declaration

    typedef unsigned short visibility;
    typedef unsigned short primitive;

    typedef bool operator_association;
    typedef unsigned char operator_fix;
    typedef struct
    {
        std::string value;
        int precedence = 0;
        int operands = 2;
        operator_association association = LEFT_OPERATOR_ASSOCATION;
        operator_fix fix = INFIX_OPERATOR;
        bool helper = false;
        bool function = false;
    } expression_operator;

    typedef struct
    {
        std::string value;
        expression_operator* operator_data = nullptr;
        int parameter_index = -1;
        function_symbol* function;
        bool call_start = false;
    } rpn_element;

    extern std::map<std::string, expression_operator> OPERATORS;
    extern std::deque<std::string> STANDARD_PUNCTUATORS;
    extern std::vector<std::string> STANDARD_KEYWORDS;
    extern std::string KEYWORD_REGEX_PATTERN;
}

#include "assembler.h"
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

    namespace specifiers
    {
        const unsigned short INVALID = -1;
        const unsigned short SEGREGATE = 0;

        std::string name(unsigned short type);
        visibility value_of(std::string name);
    }

    unsigned char is_keyword(std::string& test);

    unsigned char is_punctuator(std::string& test);

    char get_visibility_id(std::string& test);

    bool is_numerical(std::string& test);

    bool is_string_literal(std::string& test);

    bool is_number_literal(std::string& test, bool integral = false);
    
    bool is_float_literal(std::string& test);

    bool is_double_literal(std::string& test);

    std::string strip_number_literal(std::string& test);

    int get_register_size(std::string& reg);

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