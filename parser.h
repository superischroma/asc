#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <stack>

#include "symbol.h"
#include "asc.h"

namespace asc
{
    typedef unsigned char evaluation_state;
    const unsigned char STATE_NEUTRAL = 0;
    const unsigned char STATE_FOUND = 1;
    const unsigned char STATE_SYNTAX_ERROR = 2;

    class parser
    {
    private:
        evaluation_state recur_func_stack_args(syntax_node*& lcurrent, bool exp);
    public:
        // tracking variables
        syntax_node* current; // syntax token being evaluated
        assembler as; // constructor for assembly code
        std::map<std::string, std::vector<symbol*>> symbols; // symbol table
        symbol* scope; // scope of next tokens, null if global
        int branchc; // counter for branches
        int slc; // string literal counter
        int fplc; // floating point literal counter
        int dpc; // data preservation counter (how much data do we need to preserve right now?)
        int dpm; // data preservation max (how many will we need at a time)
        std::deque<stackable_element*> stack_emulation;

        parser(syntax_node* root);

        // utility
        bool parseable();
        static bool check_eof(syntax_node* node, bool silence = false);

        // eval methods

        // evaluate function headers
        evaluation_state eval_function_header(syntax_node*& lcurrent, function_symbol*& result, bool use_declaration);
        evaluation_state eval_function_header();

        // evaluate if statements
        evaluation_state eval_if_statement(syntax_node*& lcurrent);
        evaluation_state eval_if_statement();
        evaluation_state eval_while_statement(syntax_node*& lcurrent);
        evaluation_state eval_while_statement();
        evaluation_state eval_block_ending(syntax_node*& lcurrent);
        evaluation_state eval_block_ending();
        evaluation_state eval_use(syntax_node*& lcurrent);
        evaluation_state eval_use();
        evaluation_state eval_var_declaration(syntax_node*& lcurrent);
        evaluation_state eval_var_declaration();
        evaluation_state eval_expression(syntax_node*& lcurrent);
        evaluation_state eval_expression();
        evaluation_state eval_return_statement(syntax_node*& lcurrent);
        evaluation_state eval_return_statement();
        evaluation_state eval_type_construct(syntax_node*& lcurrent);
        evaluation_state eval_type_construct();

        // segments of evaluation

        // evaluate full type
        evaluation_state eval_full_type(syntax_node*& lcurrent, type_symbol*& found, bool& array);

        // value management
        int preserve_value(storage_register& location, symbol* scope = nullptr);
        int preserve_symbol(symbol* sym, symbol* scope = nullptr);
        int reserve_data_space(int size);
        storage_register& retrieve_value(storage_register& storage, bool lea = false, bool cc = false, bool sx = false);
        std::string top_location();
        void forget_top();

        // symbol table methods
        bool symbol_table_has(std::string name, symbol* scope = nullptr);
        symbol* symbol_table_get(std::string name, symbol* scope = nullptr);
        symbol* symbol_table_get_imm(std::string name, symbol* scope = nullptr);
        symbol* symbol_table_insert(std::string name, symbol* s);
        void symbol_table_delete(symbol* s);

        // utility
        symbol* get_current_function();
        symbol* floating_point_stack(int argc = 2);

        // type management
        type_symbol* get_type(std::string str);

        // destructor
        ~parser();
    };
}

#endif