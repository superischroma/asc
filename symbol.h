#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>

#include "syntax.h"

namespace asc
{
    typedef unsigned short visibility;

    typedef unsigned int symbol_variant;
    namespace symbol_variants
    {
        const symbol_variant LOCAL_VARIABLE = 0x00;
        const symbol_variant GLOBAL_VARIABLE = 0x01;
        const symbol_variant FUNCTION_VARIABLE = 0x02;
        const symbol_variant FUNCTION = 0x03;
        const symbol_variant IF_BLOCK = 0x04;
        const symbol_variant WHILE_BLOCK = 0x05;
        const symbol_variant GENERIC_BLOCK = 0x06;
        const symbol_variant STRUCTLIKE_TYPE = 0x07;
        const symbol_variant STRUCTLIKE_TYPE_MEMBER = 0x08;
        const symbol_variant OBJECT = 0x09;
        const symbol_variant PRIMITIVE = 0x0A;
        const symbol_variant INTEGRAL_PRIMITIVE = 0x0B;
        const symbol_variant UNSIGNED_INTEGRAL_PRIMITIVE = 0x0C;
        const symbol_variant FLOATING_POINT_PRIMITIVE = 0x0D;

        std::string name(symbol_variant st);
    }

    class stackable_element
    {
    public:
        bool dynamic;
    protected:
        stackable_element();
    public:
        virtual std::string to_string() = 0;
        virtual int get_size() = 0;
        std::string word();
    };

    class integral_literal: public stackable_element
    {
    public:
        int size;

        integral_literal(int size);
        std::string to_string() override;
        int get_size() override;
    };

    class storage_register: public stackable_element
    {
    public:
        std::string m_name;
        int size;

        storage_register(std::string name, int size);
        std::string to_string() override;
        int get_size() override;
        storage_register& byte_equivalent(int bs);
        bool is_fp_register();
        std::string instruction_suffix();
    };

    extern std::map<std::string, asc::storage_register> STANDARD_REGISTERS;
    storage_register& get_register(std::string& str);
    storage_register& get_register(std::string&& str);

    class type_symbol; // forward declaration of type symbol

    class symbol: public stackable_element
    {
    public:
        std::string m_name;
        type_symbol* type;
        bool array;
        symbol_variant variant;
        visibility vis;
        symbol* scope;
        syntax_node* helper;
        int offset;
        int split_b;
        bool name_identified;
        
        symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*& scope);
        symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*&& scope);
        std::string name();
        std::string location();
        std::string to_string() override;
        std::string instruction_suffix();
        int get_size() override;
    };

    class type_symbol: public symbol
    {
    public:
        std::deque<symbol*> members;
        int size;

        type_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, int size, symbol*& scope);
        type_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, int size, symbol*&& scope);
        std::string to_string() override;
        int get_size() override;
        bool is_primitive();
    };

    extern std::map<std::string, asc::type_symbol> STANDARD_TYPES;

    class function_symbol: public symbol
    {
    public:
        std::deque<symbol*> parameters;
        bool external_decl;

        function_symbol(std::string name, type_symbol* type, bool array, symbol_variant variant, visibility vis, symbol*& scope, bool external_decl);
        std::string to_string() override;
        int get_size() override;
    };
}

#endif