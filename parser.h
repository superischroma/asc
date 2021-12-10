#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <stack>

#include "syntax.h"
#include "assembler.h"
#include "logger.h"

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
        std::map<std::string, symbol*> symbols; // symbol table
        symbol* scope; // scope of next tokens, null if global
        int branchc; // counter for branches
        int strlitc;
        int dpc; // data preservation counter (how much data do we need to preserve right now?)
        int dpm; // data preservation max (how many will we need at a time)

        parser(syntax_node* root);
        bool parseable();
        bool check_eof(syntax_node* node, bool silence = false);
        evaluation_state eval_type(syntax_node*& lcurrent);
        evaluation_state eval_type();
        evaluation_state eval_visibility(syntax_node*& lcurrent);
        evaluation_state eval_visibility();
        evaluation_state eval_numeric_literal(syntax_node*& lcurrent);
        evaluation_state eval_numeric_literal();
        evaluation_state eval_exp_ending(syntax_node*& lcurrent);
        evaluation_state eval_exp_ending();
        evaluation_state eval_operator(syntax_node*& lcurrent);
        evaluation_state eval_operator();
        evaluation_state eval_function_decl(syntax_node*& lcurrent);
        evaluation_state eval_function_decl();
        evaluation_state eval_function_call(syntax_node*& lcurrent);
        evaluation_state eval_function_call();
        evaluation_state eval_ret(syntax_node*& lcurrent);
        evaluation_state eval_ret();
        evaluation_state eval_if(syntax_node*& lcurrent);
        evaluation_state eval_if();
        evaluation_state eval_hardcode(syntax_node*& lcurrent);
        evaluation_state eval_hardcode();
        evaluation_state eval_while(syntax_node*& lcurrent);
        evaluation_state eval_while();
        evaluation_state eval_block_ending(syntax_node*& lcurrent);
        evaluation_state eval_block_ending();
        evaluation_state eval_variable_decl_def(syntax_node*& lcurrent);
        evaluation_state eval_variable_decl_def();
        evaluation_state eval_use(syntax_node*& lcurrent);
        evaluation_state eval_use();
        evaluation_state eval_expression(syntax_node*& lcurrent, asc::symbol* application);
        evaluation_state eval_expression();
        int preserve_value(std::string location);
        int reserve_data_space(int size);
        void retrieve_value(int position, std::string storage);
        ~parser();
    };
}

#endif