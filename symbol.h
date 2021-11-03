#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>

namespace asc
{
    class symbol
    {
    public:
        std::string m_name;
        std::string type;
        std::string visibility;
        symbol* scope;
        int stack_m;
        int split_b;
        
        symbol(std::string name, std::string type, std::string visibility, symbol*& scope);
        std::string name();
    };
}

#endif