#include "syntax.h"
#include "util.h"

namespace asc
{
    std::map<std::string, expression_operator> OPERATORS = {
        // operators which have multiple variants follow these rules:
        // it will include its number of operands after itself in the key for it
        // it will have an 's' if it is suffix, 'p' if it is prefix, or 'i' if it is infix

        // name, precedence, operand count, association, fix

        // comma
        { ",", { ",", 1, 2, LEFT_OPERATOR_ASSOCATION, INFIX_OPERATOR, true } },
        
        // assignment operators
        { "&=", { "&=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "^=", { "^=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "|=", { "|=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "<<=", { "<<=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { ">>=", { ">>=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "*=", { "*=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "/=", { "/=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "%=", { "%=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "+=", { "-=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "+=", { "-=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },
        { "=", { "=", 2, 2, RIGHT_OPERATOR_ASSOCATION } },

        // ternary operator
        { "?", { "?", 2, 3, RIGHT_OPERATOR_ASSOCATION } },
        { ":", { ":", 2, 3, RIGHT_OPERATOR_ASSOCATION, INFIX_OPERATOR, true } },

        // arithmetic, equality, and bitwise operators
        { "||", { "||", 3, 2 } },
        { "&&", { "&&", 4, 2 } },
        { "==", { "==", 5, 2 } },
        { "!=", { "!=", 5, 2 } },
        { "<", { "<", 6, 2 } },
        { "<=", { "<=", 6, 2 } },
        { ">", { ">", 6, 2 } },
        { ">=", { ">=", 6, 2 } },
        { "<=>", { "<=>", 7, 2 } },
        { "|", { "|", 8, 2 } },
        { "^", { "^", 9, 2 } },
        { "&", { "&", 10, 2 } },
        { "<<", { "<<", 11, 2 } },
        { ">>", { ">>", 11, 2 } },
        { "+", { "+", 12, 2 } },
        { "-", { "-", 12, 2 } },
        { "*", { "*", 13, 2 } },
        { "/", { "/", 13, 2 } },
        { "%", { "%", 13, 2 } },
        // precedence level 14 reserved for pointer-to-member operator cuz ion know how it works rn lol

        // prefix-only unary operators
        { "&1p", { "&", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "*1p", { "*", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "!", { "!", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "~", { "~", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "+1p", { "+", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "-1p", { "-", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "++", { "++", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },
        { "--", { "--", 15, 1, RIGHT_OPERATOR_ASSOCATION, PREFIX_OPERATOR } },

        // object-oriented member resolution operators
        { ".", { ".", 16, 2, LEFT_OPERATOR_ASSOCATION }},
        { "->", { "->", 16, 2, LEFT_OPERATOR_ASSOCATION }},

        // suffix operators
        { "[", { "[", 16, 1, LEFT_OPERATOR_ASSOCATION, SUFFIX_OPERATOR }},
        { "]", { "]", 16, 1, LEFT_OPERATOR_ASSOCATION, SUFFIX_OPERATOR, true }},
        { "++1s", { "++", 16, 1, LEFT_OPERATOR_ASSOCATION, SUFFIX_OPERATOR } },
        { "--1s", { "--", 16, 1, LEFT_OPERATOR_ASSOCATION, SUFFIX_OPERATOR } },

        // scope resolution operator
        { "::", { "::", 17, 2, LEFT_OPERATOR_ASSOCATION } }
    };

    std::map<std::string, asc::syntax_node*> TYPES = {
        { "void", nullptr },
        { "byte", nullptr },
        { "int", nullptr },
        { "short", nullptr },
        { "int", nullptr },
        { "long", nullptr },
        { "bool", nullptr },
        { "char", nullptr },
        { "float", nullptr },
        { "double", nullptr },
        { "type", nullptr }
    };

    namespace syntax_types
    {
        std::string name(unsigned short type)
        {
            switch (type)
            {
                case 0: return "PROGRAM_BEGIN";
                case 1: return "KEYWORD";
                case 2: return "IDENTIFIER";
                case 3: return "PUNCTUATOR";
                case 4: return "CONSTANT";
                case 5: return "STRING_LITERAL";
                default: return "UNNAMED_SYNTAX_TYPE_" + std::to_string(type);
            }
        }
    }

    namespace visibilities
    {
        std::string name(unsigned short type)
        {
            switch (type)
            {
                case PUBLIC: return "PUBLIC";
                case PRIVATE: return "PRIVATE";
                case PROTECTED: return "PROTECTED";
                case LOCAL: return "LOCAL";
                default: return "UNNAMED_VISIBILITY_" + std::to_string(type);
            }
        }

        visibility value_of(std::string name)
        {
            if (name == "PUBLIC")
                return PUBLIC;
            if (name == "PRIVATE")
                return PRIVATE;
            if (name == "PROTECTED")
                return PROTECTED;
            if (name == "LOCAL")
                return LOCAL;
            return INVALID;
        }
    }

    namespace primitives
    {
        std::string name(unsigned short type)
        {
            switch (type)
            {
                case PRIMITIVE_VOID: return "PRIMITIVE_VOID";
                case PRIMITIVE_BYTE: return "PRIMITIVE_BYTE";
                case PRIMITIVE_SHORT: return "PRIMITIVE_SHORT";
                case PRIMITIVE_INT: return "PRIMITIVE_INT";
                case PRIMITIVE_LONG: return "PRIMITIVE_LONG";
                case PRIMITIVE_BOOL: return "PRIMITIVE_BOOL";
                case PRIMITIVE_CHAR: return "PRIMITIVE_CHAR";
                case PRIMITIVE_FLOAT: return "PRIMITIVE_FLOAT";
                case PRIMITIVE_DOUBLE: return "PRIMITIVE_DOUBLE";
                default: return "UNNAMED_PRIMITIVE_" + std::to_string(type);
            }
        }

        primitive from_display(std::string name)
        {
            if (name == "void")
                return PRIMITIVE_VOID;
            if (name == "byte")
                return PRIMITIVE_BYTE;
            if (name == "short")
                return PRIMITIVE_SHORT;
            if (name == "int")
                return PRIMITIVE_INT;
            if (name == "long")
                return PRIMITIVE_LONG;
            if (name == "bool")
                return PRIMITIVE_BOOL;
            if (name == "char")
                return PRIMITIVE_CHAR;
            if (name == "float")
                return PRIMITIVE_FLOAT;
            if (name == "double")
                return PRIMITIVE_DOUBLE;
            return INVALID;
        }
    }

    const char* PUNCTUATORS[] = {
        "{",
        "}",
        "(",
        ")",
        ";",
        "=",
        ",",
        "+",
        "-",
        "*",
        "/",
        "[",
        "]",
        "@"
    };

    const char* OPERATORS_OLD[] = {
        "=",
        "+",
        "-",
        "*",
        "/",
        "%"
    };

    unsigned char is_keyword(std::string& test)
    {
        std::string pattern = "\\b(void|byte|bool|char|short|int|long|float|double|signed|unsigned|return|public|private|protected|if|for|while|use|native|type|extends|object)\\b";
        std::cmatch results;
        if (std::regex_match(test.c_str(), results, std::regex(pattern), std::regex_constants::match_default))
            return results[0].str().length();
        return 0;
    }

    unsigned char is_punctuator(std::string& test)
    {
        for (int i = 0; i < sizeof(PUNCTUATORS) / sizeof(PUNCTUATORS[0]); i++)
        {
            if (test.length() < strlen(PUNCTUATORS[i]))
                continue;
            if (PUNCTUATORS[i] == test.substr(test.length() - strlen(PUNCTUATORS[i])))
                return strlen(PUNCTUATORS[i]);
        }
        return 0;
    }

    int get_operator(std::string& test)
    {
        for (int i = 0; i < sizeof(OPERATORS_OLD) / sizeof(OPERATORS_OLD[0]); i++)
        {
            if (OPERATORS_OLD[i] == test)
                return i;
        }
        return -1;
    }

    char get_visibility_id(std::string& test)
    {
        if (test == "public")
            return 0;
        if (test == "private")
            return 1;
        if (test == "protected")
            return 2;
        return -1;
    }

    bool is_numerical(std::string& test)
    {
        if (test == ".")
            return false;
        int i = 0;
        for (; test[i] == '0' ||
            test[i] == '1' ||
            test[i] == '2' ||
            test[i] == '3' ||
            test[i] == '4' ||
            test[i] == '5' ||
            test[i] == '6' ||
            test[i] == '7' ||
            test[i] == '8' ||
            test[i] == '9' ||
            test[i] == '.'; i++);
        return i == test.length();
    }

    bool is_primitive(std::string& test)
    {
        return test == "void" ||
            test == "byte" ||
            test == "bool" ||
            test == "char" ||
            test == "short" ||
            test == "int" ||
            test == "long" ||
            test == "float" ||
            test == "double";
    }

    int get_type_size(std::string& prim)
    {
        if (prim == "void" || prim == "byte" || prim == "bool")
            return 1;
        if (prim == "char" || prim == "short")
            return 2;
        if (prim == "int" || prim == "float")
            return 4;
        return 8;
    }

    int get_register_size(std::string& reg)
    {
        if (reg.length() < 2)
            return -1;
        char& first = reg[0];
        char& last = reg[reg.length() - 1];
        if (first == 'r') // 64-bit register (rax, rbx, rcx, rdx, etc...)
            return 8;
        if (first == 'e' || last == 'd') // 32-bit register (eax, ebx, ecx, edx, etc...)
            return 4;
        if (last == 'x' || last == 'i' || last == 'p' || last == 'w') // 16-bit register (ax, bx, cx, dx, etc...)
            return 2;
        if (last == 'l' || last == 'b') // 8-bit register (al, bl, cl, dl, etc...)
            return 1;
        return -1;
    }

    std::string get_word(int size)
    {
        if (size == 1)
            return "byte";
        if (size == 2)
            return "word";
        if (size == 4)
            return "dword";
        if (size == 8)
            return "qword";
        return "-1";
    }

    std::string& unwrap(std::string& sl)
    {
        if (sl.length() == 0) // not even a quote LOL
            return sl;
        if (sl[0] == '"') // beginning quote
            sl.erase(sl.begin());
        if (sl[sl.length() - 1] == '"') // end quote
            sl.erase(sl.end());
        return sl;
    }

    /* class syntax_node */

    syntax_node::syntax_node(syntax_node* next, unsigned short type, std::string value, int line)
    {
        this->next = next;
        this->type = type;
        this->value = new std::string(value);
        this->line = line;
    }

    std::string syntax_node::stringify()
    {
        return "syntax_node{type=" + syntax_types::name(type) + ", value=" + (value != nullptr ? *value : "") + ", line=" + std::to_string(line) + "}";
    }

    bool syntax_node::operator==(std::string value)
    {
        return *(this->value) == value;
    }

    bool syntax_node::operator!=(std::string value)
    {
        return *(this->value) != value;
    }

    syntax_node::~syntax_node()
    {
        delete next;
        delete value;
    }
}