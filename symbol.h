#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>

#include "syntax.h"

namespace asc
{
    typedef unsigned int symbol_type;
    namespace symbol_types
    {
        const symbol_type LOCAL_VARIABLE = 0x00;
        const symbol_type GLOBAL_VARIABLE = 0x01;
        const symbol_type FUNCTION_VARIABLE = 0x02;
        const symbol_type FUNCTION = 0x03;
        const symbol_type IF_BLOCK = 0x04;
        const symbol_type WHILE_BLOCK = 0x05;
        const symbol_type GENERIC_BLOCK = 0x06;
        const symbol_type STRUCTLIKE_TYPE = 0x07;
        const symbol_type STRUCTLIKE_TYPE_MEMBER = 0x08;
        const symbol_type OBJECT = 0x09;

        std::string name(symbol_type st);
    }

    class symbol
    {
    public:
        std::string m_name;
        std::string type;
        symbol_type s_type;
        visibility vis;
        symbol* scope;
        syntax_node* helper;
        int offset;
        int split_b;
        
        symbol(std::string name, std::string type, symbol_type s_type, visibility vis, symbol*& scope);
        symbol(std::string name, std::string type, symbol_type s_type, visibility vis, symbol*&& scope);
        std::string name();
        virtual std::string to_string();
    };

    class type_symbol: public symbol
    {
    public:
        std::vector<syntax_node*> members;
        int b_size;

        type_symbol(std::string name, std::string type, symbol_type s_type, visibility vis, symbol*& scope);
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