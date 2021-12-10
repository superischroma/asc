#include <string>

#include "symbol.h"

namespace asc
{
    namespace symbol_types
    {
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