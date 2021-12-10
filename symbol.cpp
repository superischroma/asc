#include "symbol.h"

namespace asc
{
    symbol::symbol(std::string name, std::string type, symbol_type s_type, std::string visibility, symbol*& scope)
    {
        this->m_name = name;
        this->type = type;
        this->s_type = s_type;
        this->visibility = visibility;
        this->scope = scope;
        this->offset = 0;
        this->split_b = 0;
    }

    std::string symbol::name()
    {
        if (this->split_b <= 0)
            return this->m_name;
        else
            return 'B' + std::to_string(this->split_b);
    }
}