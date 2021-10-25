#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>

namespace asc
{
    class symbol
    {
    public:
        std::string* name;
        std::string type;
        std::string visibility;
        symbol* scope;
        int stack_m;

        symbol(std::string& name, std::string type, std::string visibility, symbol*& scope)
        {
            this->name = &name;
            this->type = type;
            this->visibility = visibility;
            this->scope = scope;
            this->stack_m = 0;
        }
    };
}

#endif