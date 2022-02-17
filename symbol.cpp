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
                case OBJECT: return "OBJECT";
                case PRIMITIVE: return "PRIMITIVE";
                case FLOATING_POINT_PRIMITIVE: return "FLOATING_POINT_PRIMITIVE";
                default: return "UNNAMED_SYMBOL_TYPE_" + std::to_string(st);
            }
        }
    }

    std::map<std::string, asc::type_symbol> STANDARD_TYPES = {
        { "void", { "void", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 0, nullptr } },
        { "byte", { "byte", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 1, nullptr } },
        { "bool", { "bool", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 1, nullptr } },
        { "char", { "char", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 2, nullptr } },
        { "short", { "short", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 2, nullptr } },
        { "int", { "int", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 4, nullptr } },
        { "long", { "long", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 8, nullptr } },
        { "float", { "float", nullptr, false, symbol_variants::FLOATING_POINT_PRIMITIVE, visibilities::INVALID, 4, nullptr } },
        { "double", { "double", nullptr, false, symbol_variants::FLOATING_POINT_PRIMITIVE, visibilities::INVALID, 8, nullptr } },
    };

    std::map<std::string, asc::storage_register> STANDARD_REGISTERS = {
        { "rax", { "rax", 8 } },
        { "rbx", { "rbx", 8 } },
        { "rcx", { "rcx", 8 } },
        { "rdx", { "rdx", 8 } },
        { "rsi", { "rsi", 8 } },
        { "rdi", { "rdi", 8 } },
        { "rbp", { "rbp", 8 } },
        { "rsp", { "rsp", 8 } },
        { "r8", { "r8", 8 } },
        { "r9", { "r9", 8 } },
        { "r10", { "r10", 8 } },
        { "r11", { "r11", 8 } },
        { "r12", { "r12", 8 } },
        { "r13", { "r13", 8 } },
        { "r14", { "r14", 8 } },
        { "r15", { "r15", 8 } },
        { "eax", { "eax", 4 } },
        { "ebx", { "ebx", 4 } },
        { "ecx", { "ecx", 4 } },
        { "edx", { "edx", 4 } },
        { "esi", { "esi", 4 } },
        { "edi", { "edi", 4 } },
        { "ebp", { "ebp", 4 } },
        { "esp", { "esp", 4 } },
        { "r8d", { "r8d", 4 } },
        { "r9d", { "r9d", 4 } },
        { "r10d", { "r10d", 4 } },
        { "r11d", { "r11d", 4 } },
        { "r12d", { "r12d", 4 } },
        { "r13d", { "r13d", 4 } },
        { "r14d", { "r14d", 4 } },
        { "r15d", { "r15d", 4 } },
        { "ax", { "ax", 2 } },
        { "bx", { "bx", 2 } },
        { "cx", { "cx", 2 } },
        { "dx", { "dx", 2 } },
        { "si", { "si", 2 } },
        { "di", { "di", 2 } },
        { "bp", { "bp", 2 } },
        { "sp", { "sp", 2 } },
        { "r8w", { "r8w", 2 } },
        { "r9w", { "r9w", 2 } },
        { "r10w", { "r10w", 2 } },
        { "r11w", { "r11w", 2 } },
        { "r12w", { "r12w", 2 } },
        { "r13w", { "r13w", 2 } },
        { "r14w", { "r14w", 2 } },
        { "r15w", { "r15w", 2 } },
        { "al", { "al", 1 } },
        { "bl", { "bl", 1 } },
        { "cl", { "cl", 1 } },
        { "dl", { "dl", 1 } },
        { "sil", { "sil", 1 } },
        { "dil", { "dil", 1 } },
        { "bpl", { "bpl", 1 } },
        { "spl", { "spl", 1 } },
        { "r8b", { "r8b", 1 } },
        { "r9b", { "r9b", 1 } },
        { "r10b", { "r10b", 1 } },
        { "r11b", { "r11b", 1 } },
        { "r12b", { "r12b", 1 } },
        { "r13b", { "r13b", 1 } },
        { "r14b", { "r14b", 1 } },
        { "r15b", { "r15b", 1 } }
    };

    stackable_element::stackable_element()
    {
        this->dynamic = false;
    }

    numeric_literal::numeric_literal(int size)
    {
        this->size = size;
    }

    int numeric_literal::get_size()
    {
        return size;
    }

    std::string numeric_literal::to_string()
    {
        return "asc::numeric_literal{size=" + std::to_string(size) + '}';
    }

    storage_register::storage_register(std::string name, int size)
    {
        this->m_name = name;
        this->size = size;
    }

    int storage_register::get_size()
    {
        return size;
    }
    
    std::string storage_register::to_string()
    {
        return "asc::storage_register{name=" + m_name + ", size=" + std::to_string(size) + '}';
    }

    std::string storage_register::word()
    {
        if (size == 1)
            return "byte";
        if (size == 2)
            return "word";
        if (size == 4)
            return "dword";
        return "qword";
    }

    storage_register& storage_register::byte_equivalent(int bs)
    {
        if (size == bs)
            return *this;
        if (m_name[0] == 'r')
            return STANDARD_REGISTERS[m_name.substr(0, 3) + (size != 8 ? std::to_string(word()[0]) : "")];
        std::string result;
        if (m_name.find("sp"))
            result = "sp";
        else if (m_name.find("bp"))
            result = "bp";
        else if (m_name.find("di"))
            result = "di";
        else if (m_name.find("si"))
            result = "si";
        else if (m_name.find("d"))
            result = "d";
        else if (m_name.find("c"))
            result = "c";
        else if (m_name.find("b"))
            result = "b";
        else if (m_name.find("a"))
            result = "a";
        if (bs == 1)
            result += 'l';
        else if (bs == 2 && result[0] >= 'a' && result[0] <= 'd')
            result += 'x';
        else if (bs == 4 || bs == 8)
        {
            result = (bs == 4 ? 'e' : 'r') + result;
            if (result[1] >= 'a' && result[1] <= 'd')
                result += 'x';
        }
        return STANDARD_REGISTERS[result];
    }

    symbol::symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*& scope)
    {
        this->m_name = name;
        this->type = type;
        this->array = array;
        this->variant = variant;
        this->vis = vis;
        this->scope = scope;
        this->helper = nullptr;
        this->offset = 0;
        this->split_b = 0;
        if (asc::has_option_set(asc::args, asc::cli_options::SYMBOLIZE))
            asc::info(this->to_string());
    }

    symbol::symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*&& scope):
        symbol(name, type, array, variant, vis, scope) {}

    std::string symbol::location()
    {
        if (scope == nullptr)
            return m_name;
        else
            return asc::relative_dereference("rbp", offset);
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

    int symbol::get_size()
    {
        return type->size;
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, int size, symbol*& scope):
        symbol(name, type, array, variant, vis, scope)
    {
        this->size = size;
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, int size, symbol*&& scope):
        type_symbol(name, type, array, variant, vis, size, scope) {}

    std::string type_symbol::to_string()
    {
        std::string s = "asc::type_symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null") + ", symbol_variant=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) + ", size=" + std::to_string(size) +
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

    std::string type_symbol::word()
    {
        if (size == 1)
            return "byte";
        if (size == 2)
            return "word";
        if (size == 4)
            return "dword";
        return "qword";
    }

    function_symbol::function_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*& scope, int parameter_count):
        symbol(name, type, array, variant, vis, scope)
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