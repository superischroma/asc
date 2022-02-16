#include "symbol.h"
#include "syntax.h"
#include "asc.h"
#include "cli.h"
#include "logger.h"

namespace asc
{
    namespace symbol_variants
    {
        std::string name(symbol_variant st)
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
                case STRUCTLIKE_TYPE: return "STRUCTLIKE_TYPE";
                case STRUCTLIKE_TYPE_MEMBER: return "STRUCTLIKE_TYPE_MEMBER";
                default: return "UNNAMED_SYMBOL_TYPE_" + std::to_string(st);
            }
        }
    }

    std::map<std::string, asc::type_symbol> STANDARD_TYPES = {
        { "int", { "int", nullptr, symbol_variants::PRIMITIVE, visibilities::INVALID, nullptr } } 
    };

    symbol::symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*& scope)
    {
        this->m_name = name;
        this->type = type;
        this->variant = variant;
        this->vis = vis;
        this->scope = scope;
        this->helper = nullptr;
        this->offset = 0;
        this->split_b = 0;
        this->size = 8;
        if (asc::has_option_set(asc::args, asc::cli_options::SYMBOLIZE))
            asc::info(this->to_string());
    }

    symbol::symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*&& scope):
        symbol(name, type, variant, vis, scope) {}

    std::string symbol::location()
    {
        if (scope == nullptr)
            return m_name;
        else
            return "[rbp + " + std::to_string(offset) + ']';
    }

    std::string symbol::name()
    {
        if (this->split_b <= 0)
            return this->m_name;
        else
            return 'B' + std::to_string(this->split_b);
    }

    std::string symbol::to_string()
    {
        return "asc::symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null") + ", symbol_variant=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) +
            ", scope=" + (scope != nullptr ? scope->name() : "<global>") + '}';
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*& scope):
        symbol(name, type, variant, vis, scope)
    {
        
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*&& scope):
        type_symbol(name, type, variant, vis, scope)
    {
        
    }

    std::string type_symbol::to_string()
    {
        std::string s = "asc::type_symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null") + ", symbol_variant=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) +
            ", scope=" + (scope != nullptr ? scope->name() : "<global>") + ", members=[";
        for (int i = 0; i < members.size(); i++)
        {
            if (i != 0) s += ", ";
            s += members[i]->m_name;
        }
        s += ']';

        s += '}';
        return s;
    }

    function_symbol::function_symbol(std::string name, type_symbol*, symbol_variant variant, visibility vis, symbol*& scope, int parameter_count):
        symbol(name, type, variant, vis, scope)
    {
        this->parameter_count = parameter_count;
    }

    std::string function_symbol::to_string()
    {
        return "asc::symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null") + ", symbol_type=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) +
            ", scope=" + (scope != nullptr ? scope->name() : "<global>") + ", parameter_count=" +
            std::to_string(parameter_count) + '}';
    }
}