#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

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

        std::string name(unsigned short type)
        {
            switch (type)
            {
                case 0: return "PROGRAM_BEGIN";
                case 1: return "KEYWORD";
                case 2: return "IDENTIFIER";
                case 3: return "PUNCTUATOR";
                case 4: return "CONSTANT";
            }
            return std::to_string(type);
        }
    }

    const char* KEYWORDS[] = {
        "void",
        "byte",
        "bool",
        "char",
        "short",
        "int",
        "long",
        "float",
        "double",
        "signed",
        "unsigned",
        "return",
        "public",
        "private",
        "protected",
        "if",
        "for",
        "while"
    };

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
        "/"
    };

    const char* OPERATORS[] = {
        "=",
        "+",
        "-",
        "*",
        "/",
        "%"
    };

    int strlen(const char* str)
    {
        int length = 0;
        for (; str[length] != '\0'; length++);
        return length;
    }

    unsigned char is_keyword(std::string& test)
    {
        for (int i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++)
        {
            if (test.length() < strlen(KEYWORDS[i]))
                continue;
            if (KEYWORDS[i] == test.substr(test.length() - strlen(KEYWORDS[i])))
                return strlen(KEYWORDS[i]);
        }
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
        for (int i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); i++)
        {
            if (OPERATORS[i] == test)
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
        if (prim == "void")
            return 0;
        if (prim == "byte" || prim == "bool")
            return 1;
        if (prim == "char" || prim == "short")
            return 2;
        if (prim == "int" || prim == "float")
            return 4;
        return 8;
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

    class syntax_node
    {
    public:
        syntax_node* next;
        unsigned short type;
        std::string* value;
        int line;
        syntax_node(syntax_node* next, unsigned short type, std::string value, int line)
        {
            this->next = next;
            this->type = type;
            this->value = new std::string(value);
            this->line = line;
        }
        std::string stringify()
        {
            return "syntax_node{type=" + syntax_types::name(type) + ", value=" + (value != nullptr ? *value : "") + ", line=" + std::to_string(line) + "}";
        }
        ~syntax_node()
        {
            delete next;
            delete value;
        }
    };
}

#endif