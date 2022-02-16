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

        std::string name(symbol_variant st);
    }

    class symbol
    {
    public:
        std::string m_name;
        type_symbol* type;
        symbol_variant variant;
        visibility vis;
        symbol* scope;
        syntax_node* helper;
        int offset;
        int split_b;
        int size;
        std::string reg;
        
        symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*& scope);
        symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*&& scope);
        std::string name();
        std::string location();
        virtual std::string to_string();
    };

    /*
    typedef struct
    {
        symbol* sym;
        std::string name;
        unsigned char size;
        std::deque<symbol*>* members;
        symbol* parent;
        //std::deque<symbol*>*
    } type_properties;
    */

    class type_symbol: public symbol
    {
    public:
        std::deque<type_symbol*> members;

        type_symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*& scope);
        type_symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*&& scope);
        std::string to_string();
    };

    class function_symbol: public symbol
    {
    public:
        int parameter_count;

        function_symbol(std::string name, type_symbol* type, symbol_variant variant, visibility vis, symbol*& scope, int parameter_count);
        std::string to_string();
    };

    /*
    bool operator<(symbol& lhs, symbol& rhs)
    {
        if (lhs.m_name < rhs.m_name) return true;
        if (lhs.m_name > rhs.m_name) return false;

        return lhs.scope < rhs.scope;
    }
    */

    /*
    class expression_derivative
    {
    public:
        syntax_node* left;
        int oid;
        int parenthesis;
        syntax_node* right;

        expression_derivative(syntax_node* left, int oid, int parenthesis, syntax_node* right);
    }
    */
}

#endif