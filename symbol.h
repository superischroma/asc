#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>

namespace asc
{
    typedef unsigned int symbol_type;
    namespace symbol_types
    {
        const symbol_type LOCAL_VARIABLE = 0x00;
        const symbol_type GLOBAL_VARIABLE = 0x01;
        const symbol_type FUNCTION_VARIABLE = 0x02;
        const symbol_type FUNCTION = 0x03;
        const symbol_type IF_BLOCK = 0x04;
        const symbol_type WHILE_BLOCK = 0x05;
        const symbol_type GENERIC_BLOCK = 0x06;

        std::string name(symbol_type st)
        {
            switch (st)
            {
                case LOCAL_VARIABLE: return "VARIABLE";
                case GLOBAL_VARIABLE: return "GLOBAL_VARIABLE";
                case FUNCTION_VARIABLE: return "FUNCTION_VARIABLE";
                case FUNCTION: return "FUNCTION";
                case IF_BLOCK: return "IF_BLOCK";
                case WHILE_BLOCK: return "WHILE_BLOCK";
                case GENERIC_BLOCK: return "GENERIC_BLOCK";
                default: return "UNNAMED_SYMBOL_TYPE_" + std::to_string(st);
            }
        }
    }

    class symbol
    {
    public:
        std::string m_name;
        std::string type;
        symbol_type s_type;
        std::string visibility;
        symbol* scope;
        int offset;
        int split_b;
        
        symbol(std::string name, std::string type, symbol_type s_type, std::string visibility, symbol*& scope);
        std::string name();
    };
}

#endif