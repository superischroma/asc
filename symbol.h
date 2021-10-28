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
        
        symbol(std::string name, std::string type, std::string visibility, symbol*& scope)
        {
            this->m_name = name;
            this->type = type;
            this->visibility = visibility;
            this->scope = scope;
            this->stack_m = 0;
            this->split_b = 0;
        }

        std::string name()
        {
            if (this->split_b <= 0)
                return this->m_name;
            else
                return 'B' + std::to_string(this->split_b);
        }
    };
}

#endif