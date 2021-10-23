#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>

namespace asc
{
    class symbol
    {
    public:
        std::string* name;
        std::string* type;
        symbol* scope;

        symbol(std::string& name, std::string& type, symbol*& scope)
        {
            this->name = &name;
            this->type = &type;
            this->scope = scope;
        }
    };
}

#endif