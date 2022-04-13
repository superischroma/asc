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
                case INTEGRAL_PRIMITIVE: return "INTEGRAL_PRIMITIVE";
                case UNSIGNED_INTEGRAL_PRIMITIVE: return "UNSIGNED_INTEGRAL_PRIMITIVE";
                default: return "UNNAMED_SYMBOL_TYPE_" + std::to_string(st);
            }
        }
    }

    std::map<std::string, asc::type_symbol> STANDARD_TYPES = {
        { "void", { "void", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 0, nullptr, nullptr } },
        { "byte", { "byte", nullptr, false, symbol_variants::INTEGRAL_PRIMITIVE, visibilities::INVALID, 1, nullptr, nullptr } },
        { "ubyte", { "ubyte", nullptr, false, symbol_variants::UNSIGNED_INTEGRAL_PRIMITIVE, visibilities::INVALID, 1, nullptr, nullptr } },
        { "bool", { "bool", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 1, nullptr, nullptr } },
        { "char", { "char", nullptr, false, symbol_variants::PRIMITIVE, visibilities::INVALID, 1, nullptr, nullptr } },
        { "sint", { "sint", nullptr, false, symbol_variants::INTEGRAL_PRIMITIVE, visibilities::INVALID, 2, nullptr, nullptr } },
        { "usint", { "usint", nullptr, false, symbol_variants::UNSIGNED_INTEGRAL_PRIMITIVE, visibilities::INVALID, 2, nullptr, nullptr } },
        { "int", { "int", nullptr, false, symbol_variants::INTEGRAL_PRIMITIVE, visibilities::INVALID, 4, nullptr, nullptr } },
        { "uint", { "uint", nullptr, false, symbol_variants::UNSIGNED_INTEGRAL_PRIMITIVE, visibilities::INVALID, 4, nullptr, nullptr } },
        { "lint", { "lint", nullptr, false, symbol_variants::INTEGRAL_PRIMITIVE, visibilities::INVALID, 8, nullptr, nullptr } },
        { "ulint", { "ulint", nullptr, false, symbol_variants::UNSIGNED_INTEGRAL_PRIMITIVE, visibilities::INVALID, 8, nullptr, nullptr } },
        { "real", { "real", nullptr, false, symbol_variants::FLOATING_POINT_PRIMITIVE, visibilities::INVALID, 4, nullptr, nullptr } },
        { "lreal", { "lreal", nullptr, false, symbol_variants::FLOATING_POINT_PRIMITIVE, visibilities::INVALID, 8, nullptr, nullptr } },
    };

    std::map<std::string, std::shared_ptr<asc::storage_register>> STANDARD_REGISTERS = {
        { "rax", std::shared_ptr<storage_register>(new storage_register{ "rax", 8 }) },
        { "rbx", std::shared_ptr<storage_register>(new storage_register{ "rbx", 8 }) },
        { "rcx", std::shared_ptr<storage_register>(new storage_register{ "rcx", 8 }) },
        { "rdx", std::shared_ptr<storage_register>(new storage_register{ "rdx", 8 }) },
        { "rsi", std::shared_ptr<storage_register>(new storage_register{ "rsi", 8 }) },
        { "rdi", std::shared_ptr<storage_register>(new storage_register{ "rdi", 8 }) },
        { "rbp", std::shared_ptr<storage_register>(new storage_register{ "rbp", 8 }) },
        { "rsp", std::shared_ptr<storage_register>(new storage_register{ "rsp", 8 }) },
        { "r8", std::shared_ptr<storage_register>(new storage_register{ "r8", 8 }) },
        { "r9", std::shared_ptr<storage_register>(new storage_register{ "r9", 8 }) },
        { "r10", std::shared_ptr<storage_register>(new storage_register{ "r10", 8 }) },
        { "r11", std::shared_ptr<storage_register>(new storage_register{ "r11", 8 }) },
        { "r12", std::shared_ptr<storage_register>(new storage_register{ "r12", 8 }) },
        { "r13", std::shared_ptr<storage_register>(new storage_register{ "r13", 8 }) },
        { "r14", std::shared_ptr<storage_register>(new storage_register{ "r14", 8 }) },
        { "r15", std::shared_ptr<storage_register>(new storage_register{ "r15", 8 }) },
        { "eax", std::shared_ptr<storage_register>(new storage_register{ "eax", 4 }) },
        { "ebx", std::shared_ptr<storage_register>(new storage_register{ "ebx", 4 }) },
        { "ecx", std::shared_ptr<storage_register>(new storage_register{ "ecx", 4 }) },
        { "edx", std::shared_ptr<storage_register>(new storage_register{ "edx", 4 }) },
        { "esi", std::shared_ptr<storage_register>(new storage_register{ "esi", 4 }) },
        { "edi", std::shared_ptr<storage_register>(new storage_register{ "edi", 4 }) },
        { "ebp", std::shared_ptr<storage_register>(new storage_register{ "ebp", 4 }) },
        { "esp", std::shared_ptr<storage_register>(new storage_register{ "esp", 4 }) },
        { "r8d", std::shared_ptr<storage_register>(new storage_register{ "r8d", 4 }) },
        { "r9d", std::shared_ptr<storage_register>(new storage_register{ "r9d", 4 }) },
        { "r10d", std::shared_ptr<storage_register>(new storage_register{ "r10d", 4 }) },
        { "r11d", std::shared_ptr<storage_register>(new storage_register{ "r11d", 4 }) },
        { "r12d", std::shared_ptr<storage_register>(new storage_register{ "r12d", 4 }) },
        { "r13d", std::shared_ptr<storage_register>(new storage_register{ "r13d", 4 }) },
        { "r14d", std::shared_ptr<storage_register>(new storage_register{ "r14d", 4 }) },
        { "r15d", std::shared_ptr<storage_register>(new storage_register{ "r15d", 4 }) },
        { "ax", std::shared_ptr<storage_register>(new storage_register{ "ax", 2 }) },
        { "bx", std::shared_ptr<storage_register>(new storage_register{ "bx", 2 }) },
        { "cx", std::shared_ptr<storage_register>(new storage_register{ "cx", 2 }) },
        { "dx", std::shared_ptr<storage_register>(new storage_register{ "dx", 2 }) },
        { "si", std::shared_ptr<storage_register>(new storage_register{ "si", 2 }) },
        { "di", std::shared_ptr<storage_register>(new storage_register{ "di", 2 }) },
        { "bp", std::shared_ptr<storage_register>(new storage_register{ "bp", 2 }) },
        { "sp", std::shared_ptr<storage_register>(new storage_register{ "sp", 2 }) },
        { "r8w", std::shared_ptr<storage_register>(new storage_register{ "r8w", 2 }) },
        { "r9w", std::shared_ptr<storage_register>(new storage_register{ "r9w", 2 }) },
        { "r10w", std::shared_ptr<storage_register>(new storage_register{ "r10w", 2 }) },
        { "r11w", std::shared_ptr<storage_register>(new storage_register{ "r11w", 2 }) },
        { "r12w", std::shared_ptr<storage_register>(new storage_register{ "r12w", 2 }) },
        { "r13w", std::shared_ptr<storage_register>(new storage_register{ "r13w", 2 }) },
        { "r14w", std::shared_ptr<storage_register>(new storage_register{ "r14w", 2 }) },
        { "r15w", std::shared_ptr<storage_register>(new storage_register{ "r15w", 2 }) },
        { "al", std::shared_ptr<storage_register>(new storage_register{ "al", 1 }) },
        { "bl", std::shared_ptr<storage_register>(new storage_register{ "bl", 1 }) },
        { "cl", std::shared_ptr<storage_register>(new storage_register{ "cl", 1 }) },
        { "dl", std::shared_ptr<storage_register>(new storage_register{ "dl", 1 }) },
        { "sil", std::shared_ptr<storage_register>(new storage_register{ "sil", 1 }) },
        { "dil", std::shared_ptr<storage_register>(new storage_register{ "dil", 1 }) },
        { "bpl", std::shared_ptr<storage_register>(new storage_register{ "bpl", 1 }) },
        { "spl", std::shared_ptr<storage_register>(new storage_register{ "spl", 1 }) },
        { "r8b", std::shared_ptr<storage_register>(new storage_register{ "r8b", 1 }) },
        { "r9b", std::shared_ptr<storage_register>(new storage_register{ "r9b", 1 }) },
        { "r10b", std::shared_ptr<storage_register>(new storage_register{ "r10b", 1 }) },
        { "r11b", std::shared_ptr<storage_register>(new storage_register{ "r11b", 1 }) },
        { "r12b", std::shared_ptr<storage_register>(new storage_register{ "r12b", 1 }) },
        { "r13b", std::shared_ptr<storage_register>(new storage_register{ "r13b", 1 }) },
        { "r14b", std::shared_ptr<storage_register>(new storage_register{ "r14b", 1 }) },
        { "r15b", std::shared_ptr<storage_register>(new storage_register{ "r15b", 1 }) },

        // floating point registers
        { "xmm0", std::shared_ptr<fp_register>(new fp_register{ "xmm0", 16 }) },
        { "xmm1", std::shared_ptr<fp_register>(new fp_register{ "xmm1", 16 }) },
        { "xmm2", std::shared_ptr<fp_register>(new fp_register{ "xmm2", 16 }) },
        { "xmm3", std::shared_ptr<fp_register>(new fp_register{ "xmm3", 16 }) },
        { "xmm4", std::shared_ptr<fp_register>(new fp_register{ "xmm4", 16 }) },
        { "xmm5", std::shared_ptr<fp_register>(new fp_register{ "xmm5", 16 }) },
        { "xmm6", std::shared_ptr<fp_register>(new fp_register{ "xmm6", 16 }) },
        { "xmm7", std::shared_ptr<fp_register>(new fp_register{ "xmm7", 16 }) },
        { "xmm8", std::shared_ptr<fp_register>(new fp_register{ "xmm8", 16 }) },
        { "xmm9", std::shared_ptr<fp_register>(new fp_register{ "xmm9", 16 }) },
        { "xmm10", std::shared_ptr<fp_register>(new fp_register{ "xmm10", 16 }) },
        { "xmm11", std::shared_ptr<fp_register>(new fp_register{ "xmm11", 16 }) },
        { "xmm12", std::shared_ptr<fp_register>(new fp_register{ "xmm12", 16 }) },
        { "xmm13", std::shared_ptr<fp_register>(new fp_register{ "xmm13", 16 }) },
        { "xmm14", std::shared_ptr<fp_register>(new fp_register{ "xmm14", 16 }) },
        { "xmm15", std::shared_ptr<fp_register>(new fp_register{ "xmm15", 16 }) }
    };

    std::string stackable_element::word()
    {
        if (get_size() == 1)
            return "byte";
        if (get_size() == 2)
            return "word";
        if (get_size() == 4)
            return "dword";
        return "qword";
    }

    storage_register& get_register(std::string& str)
    {
        return *(STANDARD_REGISTERS.at(str));
    }

    storage_register& get_register(std::string&& str)
    {
        return get_register(str);
    }

    fp_register::fp_register(std::string name, int size): storage_register(name, size) {}

    std::string fp_register::to_string()
    {
        return "fp_register{name=" + m_name + ", size=" + std::to_string(size) + '}';
    }

    std::string fp_register::word()
    {
        if (effective_sizes.empty())
            return storage_register::word();
        return asc::word(effective_sizes.top());
    }

    stackable_element::stackable_element()
    {
        this->dynamic = false;
    }

    integral_literal::integral_literal(int size)
    {
        this->size = size;
    }

    int integral_literal::get_size()
    {
        return size;
    }

    std::string integral_literal::to_string()
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

    storage_register& storage_register::byte_equivalent(int bs)
    {
        if (is_fp_register() || bs == 16)
            return *this;
        if (size == bs)
            return *this;
        if (m_name[0] == 'r' && m_name[1] >= '0' && m_name[1] <= '9')
        {
            char end = '\0';
            if (bs == 1)
                end = 'b';
            else if (bs == 2)
                end = 'w';
            else if (bs == 4)
                end = 'd';
            return get_register(m_name.substr(0, m_name[1] == '1' ? 3 : 2) + (bs != 8 ? std::string() + end : ""));
        }
        std::string result;
        if (m_name.find("sp") != std::string::npos)
            result = "sp";
        else if (m_name.find("bp") != std::string::npos)
            result = "bp";
        else if (m_name.find("di") != std::string::npos)
            result = "di";
        else if (m_name.find("si") != std::string::npos)
            result = "si";
        else if (m_name.find("d") != std::string::npos)
            result = "d";
        else if (m_name.find("c") != std::string::npos)
            result = "c";
        else if (m_name.find("b") != std::string::npos)
            result = "b";
        else if (m_name.find("a") != std::string::npos)
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
        return get_register(result);
    }

    bool storage_register::is_fp_register()
    {
        return m_name.find("xmm") != std::string::npos;
    }

    std::string storage_register::instruction_suffix()
    {
        return is_fp_register() ? (get_size() == 4 ? "ss" : "sd") : "";
    }

    symbol::symbol(std::string name, type_symbol* type, int pointer, symbol_variant variant, visibility vis, symbol* ns, symbol*& scope)
    {
        this->m_name = name;
        this->type = type;
        this->pointer = pointer;
        this->variant = variant;
        this->vis = vis;
        this->scope = scope;
        this->ns = ns;
        this->helper = nullptr;
        this->offset = 0;
        this->split_b = 0;
        this->name_identified = false;
        if (asc::has_option_set(asc::args, asc::cli_options::SYMBOLIZE))
            asc::info(this->to_string());
    }

    symbol::symbol(std::string name, type_symbol* type, int pointer, symbol_variant variant, visibility vis, symbol* ns, symbol*&& scope):
        symbol(name, type, pointer, variant, vis, ns, scope) {}

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
        std::string str = "asc::symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null");
        for (int i = 0; i < pointer; i++) str += '*';
        str += ", symbol_variant=" + asc::symbol_variants::name(variant) + ", visibility=" +
            visibilities::name(vis) + ", scope=" + (scope != nullptr ? scope->name() : "<global>") + '}';
        return str;
    }

    int symbol::get_size()
    {
        return !pointer ? type->size : 8;
    }

    std::string symbol::instruction_suffix()
    {
        return is_floating_point() && !pointer ? (get_size() == 4 ? "ss" : "sd") : "";
    }

    bool symbol::is_floating_point()
    {
        auto v = dynamic_cast<type_symbol*>(this) ? variant : type->variant;
        return v == symbol_variants::FLOATING_POINT_PRIMITIVE;
    }

    bool symbol::operator==(symbol& rhs)
    {
        return this->m_name == rhs.m_name;
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, int pointer, symbol_variant variant, visibility vis, int size, symbol* ns, symbol*& scope):
        symbol(name, type, pointer, variant, vis, ns, scope)
    {
        this->size = size;
    }

    type_symbol::type_symbol(std::string name, type_symbol* type, int pointer, symbol_variant variant, visibility vis, int size, symbol* ns, symbol*&& scope):
        type_symbol(name, type, pointer, variant, vis, size, ns, scope) {}
    
    int type_symbol::get_size()
    {
        return this->size;
    }

    std::string type_symbol::to_string()
    {
        std::string s = "asc::type_symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null");
        for (int i = 0; i < pointer; i++) s += '*';
        s += ", symbol_variant=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) + ", size=" + std::to_string(size) +
            ", scope=" + (scope != nullptr ? scope->name() : "<global>") + ", members=[";
        for (int i = 0; i < members.size(); i++)
        {
            if (i != 0) s += ", ";
            s += members[i]->type->m_name + asc::pointers(members[i]->pointer) + ' ' + members[i]->m_name;
        }
        s += ']';

        s += '}';
        return s;
    }

    bool type_symbol::is_primitive()
    {
        std::string n = m_name;
        return std::find_if(STANDARD_TYPES.begin(), STANDARD_TYPES.end(),
            [n](std::pair<std::string, asc::symbol> pair) -> bool
            {
                return n == pair.second.m_name;
            }) != STANDARD_TYPES.end();
    }

    // Calculates the full internal size of the type
    int type_symbol::calc_size()
    {
        int sz = 0;
        for (auto* member : this->members) sz += member->get_size();
        return sz;
    }

    // Calculates the memory offset of a type member from a pointer
    int type_symbol::calc_member_offset(symbol* member)
    {
        int off = 0;
        for (auto* m : this->members)
        {
            if (m == member)
                break;
            off += m->get_size();
        }
        return off;
    }

    function_symbol::function_symbol(std::string name, type_symbol* type, int pointer, symbol_variant variant, visibility vis, symbol* ns, symbol*& scope, bool external_decl):
        symbol(name, type, pointer, variant, vis, ns, scope)
    {
        this->external_decl = external_decl;
    }

    std::string function_symbol::to_string()
    {
        std::string str = "asc::function_symbol{name=" + m_name + ", type=" + (type != nullptr ? type->m_name : "null");
        for (int i = 0; i < pointer; i++) str += '*';
        str += ", symbol_type=" +
            asc::symbol_variants::name(variant) + ", visibility=" + visibilities::name(vis) +
            ", scope=" + (scope != nullptr ? scope->name() : "<global>") + ", parameters=[";
        for (int i = 0; i < parameters.size(); i++)
        {
            if (i != 0)
                str += ", ";
            str += parameters[i]->type->m_name;
            for (int j = 0; j < parameters[i]->pointer; j++) str += '*';
            str += ' ' + parameters[i]->m_name;
        }
        str += std::string("], external_decl=") + (this->external_decl ? "true" : "false");
        str += '}';
        return str;
    }

    int function_symbol::get_size()
    {
        return !pointer ? type->size : 8;
    }

    reference_element::reference_element(int offset, bool fp, type_symbol* type, int pointer)
    {
        this->offset = offset;
        this->fp = fp;
        this->type = type;
        this->pointer = pointer;
    }

    std::string reference_element::to_string()
    {
        return "asc::reference_element{type=" + type->m_name + asc::pointers(pointer) + ", offset=" + std::to_string(offset) + 
            ", fp=" + asc::to_string(fp) + '}';
    }

    int reference_element::get_size()
    {
        return !pointer ? type->size : 8;
    }

    std::string word(int size)
    {
        if (size == 1)
            return "byte";
        if (size == 2)
            return "word";
        if (size == 4)
            return "dword";
        if (size == 8)
            return "qword";
        return "";
    }

    int size(std::string& word)
    {
        if (word == "byte") 
            return 1;
        if (word == "word") 
            return 2;
        if (word == "dword")
            return 4;
        if (word == "qword")
            return 8;
        return -1;
    }

    int compare(std::string& w1, std::string& w2)
    {
        int s1 = size(w1), s2 = size(w2);
        if (s1 > s2)
            return 1;
        if (s1 < s2)
            return -1;
        return 0;
    }

    int compare(std::string&& w1, std::string&& w2)
    {
        return compare(w1, w2);
    }

    int compare(std::string& w1, std::string&& w2)
    {
        return compare(w1, w2);
    }
}