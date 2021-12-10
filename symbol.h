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

        std::string name(symbol_type st);
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