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
        evaluation_state recur_func_stack_args(syntax_node*& lcurrent)
        {
            if (lcurrent == nullptr)
            {
                asc::err("unexpected end to argument passing");
                return STATE_SYNTAX_ERROR;
            }
            if (*(lcurrent->value) == ")") // finished
            {
                lcurrent = lcurrent->next; // move lcurrent to its proper location
                return STATE_FOUND;
            }
            evaluation_state es = recur_func_stack_args(lcurrent->next);
            if (*(lcurrent->value) == ",")
                return STATE_NEUTRAL;
            if (es == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            evaluation_state ev_ex = eval_expression(lcurrent, nullptr); // eval for expression of current arg and store in rax
            if (ev_ex == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (ev_ex == STATE_NEUTRAL)
            {
                asc::err("expected expression", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            as.instruct(*(scope->name), "push rax");
            return es;
        }
    public:
        // tracking variables
        syntax_node* current; // syntax token being evaluated
        assembler as; // constructor for assembly code
        std::map<std::string, symbol*> symbols; // symbol table
        symbol* scope; // scope of next tokens, null if global
        parser(syntax_node* root)
        {
            this->current = root;
        }

        bool parseable()
        {
            return current != nullptr;
        }

        bool check_eof(syntax_node* node, bool silence = false)
        {
            if (node == nullptr)
            {
                if (!silence)
                    asc::err("unexpected end of file");
                return true;
            }
            return false;
        }

        evaluation_state eval_type(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return asc::is_primitive(*(lcurrent->value));
        }

        evaluation_state eval_type()
        {
            syntax_node* current = this->current;
            return eval_type(current);
        }

        evaluation_state eval_visibility(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return asc::get_visibility_id(*(lcurrent->value)) != -1;
        }

        evaluation_state eval_visibility()
        {
            syntax_node* current = this->current;
            return eval_visibility(current);
        }

        evaluation_state eval_numeric_literal(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return asc::is_numerical(*(lcurrent->value));
        }

        evaluation_state eval_numeric_literal()
        {
            syntax_node* current = this->current;
            return eval_numeric_literal(current);
        }

        evaluation_state eval_exp_ending(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            std::string& value = *(lcurrent->value);
            return value == ";" || value == "," || value == ")";
        }

        evaluation_state eval_exp_ending()
        {
            syntax_node* current = this->current;
            return eval_exp_ending(current);
        }

        evaluation_state eval_operator(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return asc::get_operator(*(lcurrent->value)) != -1;
        }

        evaluation_state eval_operator()
        {
            syntax_node* current = this->current;
            return eval_operator(current);
        }

        evaluation_state eval_function_decl(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            // errors will be thrown later on once we CONFIRM this is supposed to be a function declaration
            evaluation_state v_state = eval_visibility(lcurrent);
            std::string v = "public"; // default to public
            if (v_state == STATE_FOUND) // if a specifier was found, add it
                v = *(lcurrent->value);
            if (v_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            std::cout << 'v' << std::endl;
            if (v_state != STATE_NEUTRAL)
            {
                lcurrent = lcurrent->next;
                if (check_eof(lcurrent, true))
                    return STATE_NEUTRAL;
            }
            int t_line = lcurrent->line;
            evaluation_state t_state = eval_type(lcurrent);
            if (t_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            std::string& t = *(lcurrent->value);
            int t_size = asc::get_type_size(t);
            std::cout << 't' << std::endl;
            // at this point, it could still be a variable definition/declaration, so let's continue
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            int i_line = lcurrent->line;
            std::string& identifier = *(lcurrent->value); // get the identifier that MIGHT be there
            std::cout << 'i' << std::endl;
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) != "(") // if there is no parenthesis, it's confirmed that this is not a variable declaration
                return STATE_NEUTRAL; // return a neutral state, indicating no change
            // now let's throw some errors
            if (t_state == STATE_NEUTRAL) // if there was no type specifier, throw an error
            {
                asc::err("type specifier expected", t_line);
                return STATE_SYNTAX_ERROR;
            }
            asc::symbol*& function_symbol = symbols[identifier]; // get reference to the current symbol with this identifier
            if (function_symbol != nullptr) // if symbol already exists in this scope
            {
                asc::err("symbol is already defined", i_line);
                return STATE_SYNTAX_ERROR;
            }
            function_symbol = new asc::symbol(identifier, t, v, scope);
            std::cout << "defined " << t << ' ' << identifier << " as a function" << std::endl;
            for (int c = 1, s = 8; true; c++) // loop until we're at the end of the declaration, this is an infinite loop to make code smoother
            {
                lcurrent = lcurrent->next; // first, the argument type
                if (check_eof(lcurrent))
                    return STATE_SYNTAX_ERROR;
                if (*(lcurrent->value) == ")") // if there are no more arguments, leave the loop
                    break;
                int at_line = lcurrent->line;
                evaluation_state at_state = eval_type(lcurrent);
                if (at_state == STATE_SYNTAX_ERROR)
                    return STATE_SYNTAX_ERROR;
                std::string& at = *(lcurrent->value);
                if (at_state == STATE_NEUTRAL) // if there was no type specifier, throw an error
                {
                    asc::err("type specifier expected for argument " + c, at_line);
                    return STATE_SYNTAX_ERROR;
                }
                int at_size = asc::get_type_size(at);
                lcurrent = lcurrent->next; // second, the argument identifier
                if (check_eof(lcurrent))
                    return STATE_SYNTAX_ERROR;
                int ai_line = lcurrent->line;
                std::string& a_identifier = *(lcurrent->value); // get the identifier that MIGHT be there
                asc::symbol*& a_symbol = symbols[a_identifier]; // get reference to the current symbol with this identifier
                if (a_symbol != nullptr) // if symbol already exists in this scope
                {
                    asc::err("symbol is already defined", ai_line);
                    return STATE_SYNTAX_ERROR;
                }
                a_symbol = new asc::symbol(a_identifier, at, "public", function_symbol);
                a_symbol->stack_m = s += 8;
                if (c <= 4)
                    as.instruct(*(function_symbol->name), "mov " + get_word(at_size) + " [rbp + " + std::to_string(a_symbol->stack_m) + "], " + asc::resolve_register(ARG_REGISTER_SEQUENCE[c - 1], at_size));
                std::cout << "defined " << at << ' ' << a_identifier << " as a function argument for " << identifier << std::endl;
                lcurrent = lcurrent->next; // lastly, what's next?
                if (check_eof(lcurrent))
                    return STATE_SYNTAX_ERROR;
                if (*(lcurrent->value) == ")") // if there are no more arguments, leave the loop
                    break;
                if (*(lcurrent->value) != ",") // if there are more args and the next is not a comma
                {
                    asc::err("unexpected end to argument listing", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
            }
            // after argument listing, scope into function if syntax is correct
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            if (*(lcurrent->value) != "{") // if the syntax is not right
            {
                asc::err("expected a left curly brace to start function", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            scope = function_symbol; // scope into function
            lcurrent = lcurrent->next; // move into the function
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            current = lcurrent; // move member current to its proper location
            return STATE_FOUND; // finally, return the proper state
        }

        evaluation_state eval_function_decl()
        {
            syntax_node* current = this->current;
            return eval_function_decl(current);
        }

        evaluation_state eval_function_call(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            int i_line = lcurrent->line;
            std::string& identifier = *(lcurrent->value); // get the identifier that MIGHT be there
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) != "(") // if there is no parenthesis, it's confirmed that this is not a function call
                return STATE_NEUTRAL; // return a neutral state, indicating no change
            asc::symbol*& function_symbol = symbols[identifier]; // get reference to the current symbol with this identifier
            if (function_symbol == nullptr) // if symbol does not exist in this scope
            {
                asc::err("a");
                asc::err("symbol is not defined", i_line);
                return STATE_SYNTAX_ERROR;
            }
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            /*
            if (*(lcurrent->value) == ")") // 0-arg function
            {
                lcurrent = lcurrent->next;
                if (check_eof(lcurrent))
                    return STATE_SYNTAX_ERROR;
                if (eval_exp_ending(lcurrent) != STATE_FOUND)
                {
                    asc::err("expected semicolon", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
                lcurrent = lcurrent->next; // move past semicolon
                current = lcurrent;
                return STATE_FOUND;
            }
            */
            as.instruct(*(scope->name), "push rcx"); // make sure none of these values
            as.instruct(*(scope->name), "push rdx"); // are modified during argument passing
            as.instruct(*(scope->name), "push r8");
            as.instruct(*(scope->name), "push r9");

            as.instruct(*(scope->name), "push rbp");
            as.instruct(*(scope->name), "mov rbp, rsp"); // preserve current stack frame
            as.instruct(*(scope->name), "sub rsp, 32"); // alloc 32 bytes of shadow space
            for (int i = 0; lcurrent != nullptr && eval_exp_ending(lcurrent) != STATE_FOUND && i < sizeof(ARG_REGISTER_SEQUENCE) / sizeof(ARG_REGISTER_SEQUENCE[0]); i++)
            {
                evaluation_state ev_ex = eval_expression(lcurrent, nullptr); // eval for expression of current arg and store in rax
                if (ev_ex == STATE_SYNTAX_ERROR)
                    return STATE_SYNTAX_ERROR;
                if (ev_ex == STATE_NEUTRAL)
                {
                    asc::err("expected expression", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
                as.instruct(*(scope->name), "mov " + std::string(ARG_REGISTER_SEQUENCE[i]) + ", rax");
            }
            if (eval_exp_ending(lcurrent) != STATE_FOUND) // if there are still more arguments
            {
                if (recur_func_stack_args(lcurrent) == STATE_SYNTAX_ERROR) // iterate through stack args and push them
                    return STATE_SYNTAX_ERROR;
            }
            as.instruct(*(scope->name), "call " + identifier); // call the function

            as.instruct(*(scope->name), "mov rsp, rbp"); // restore the stack to its original state
            as.instruct(*(scope->name), "pop rbp");

            as.instruct(*(scope->name), "pop r9"); // make sure none of these values
            as.instruct(*(scope->name), "pop r8"); // are modified during argument passing
            as.instruct(*(scope->name), "pop rdx");
            as.instruct(*(scope->name), "pop rcx");
            if (lcurrent == nullptr) // unexpected end to arguments
            {
                asc::err("unexpected end to argument passing", i_line);
                return STATE_SYNTAX_ERROR;
            }
            lcurrent = lcurrent->next; // move past semicolon
            current = lcurrent;
            if (lcurrent != nullptr && *(lcurrent->value) == ";")
            {
                lcurrent = lcurrent->next; // move past semicolon
                current = lcurrent;
            }
            return STATE_FOUND;
        }

        evaluation_state eval_function_call()
        {
            syntax_node* current = this->current;
            return eval_function_call(current);
        }

        evaluation_state eval_ret(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) != "return") // not a return statement
                return STATE_NEUTRAL;
            return eval_expression(lcurrent = lcurrent->next, nullptr);
        }

        evaluation_state eval_ret()
        {
            syntax_node* current = this->current;
            return eval_ret(current);
        }

        evaluation_state eval_block_ending(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) != "}") // not a function ending
                return STATE_NEUTRAL;
            scope = scope->scope; // scope out of function
            lcurrent = lcurrent->next;
            current = lcurrent;
            return STATE_FOUND;
        }

        evaluation_state eval_block_ending()
        {
            syntax_node* current = this->current;
            return eval_block_ending(current);
        }

        evaluation_state eval_variable_decl_def(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            // errors will be thrown later on once we CONFIRM this is supposed to be a function declaration
            evaluation_state v_state = eval_visibility(lcurrent);
            std::cout << "eval var dec def vis: " << *(lcurrent->value) << std::endl;
            std::string v = "public"; // default to public
            if (v_state == STATE_FOUND) // if a specifier was found, add it
                v = *(lcurrent->value);
            if (v_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (v_state != STATE_NEUTRAL)
            {
                lcurrent = lcurrent->next;
                if (check_eof(lcurrent, true))
                    return STATE_NEUTRAL;
            }
            int t_line = lcurrent->line;
            evaluation_state t_state = eval_type(lcurrent);
            if (t_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            std::string& t = *(lcurrent->value);
            int t_size = asc::get_type_size(t);
            if (t_state != STATE_NEUTRAL)
            {
                lcurrent = lcurrent->next;
                if (check_eof(lcurrent, true))
                    return STATE_NEUTRAL;
            }
            int i_line = lcurrent->line;
            std::string& identifier = *(lcurrent->value); // get the identifier that MIGHT be there
            std::cout << "var decl def: " << v << ' ' << t << ' ' << identifier << std::endl;
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) == ";") // declaration or doing nothing with it (why)
            {
                asc::symbol*& var_symbol = symbols[identifier];
                if (t_state == STATE_NEUTRAL) // why...
                {
                    if (var_symbol == nullptr) // IT DOESN'T EVEN EXIST LOL
                    {
                        asc::err("symbol is not defined", lcurrent->line);
                        return STATE_SYNTAX_ERROR;
                    }
                    lcurrent = lcurrent->next; // move it along after the semicolon
                    current = lcurrent; // and bring current up to speed
                    return STATE_FOUND;
                }
                var_symbol = new asc::symbol(identifier, t, v, scope); // define symbol
                if (scope != nullptr) // if we're not in global scope
                {
                    as.alloc_delta(*(scope->name), t_size);
                    var_symbol->stack_m = scope->stack_m -= t_size; // set stack location
                }
                std::cout << "declared " << t << ' ' << identifier << " without definition" << std::endl;
                lcurrent = lcurrent->next; // move it along after the semicolon
                current = lcurrent; // and bring current up to speed
                return STATE_FOUND;
            }
            if (*(lcurrent->value) != "=") // confirming it is NOT a variable assignment
                return STATE_NEUTRAL;
            asc::symbol*& var_symbol = symbols[identifier];
            if (t_state == STATE_NEUTRAL && var_symbol == nullptr) // if attempting reassignment, but symbol doesn't exist
            {
                asc::err("symbol is not defined", i_line);
                return STATE_SYNTAX_ERROR;
            }
            if (t_state != STATE_NEUTRAL)
            {
                var_symbol = new asc::symbol(identifier, t, v, scope);
                if (scope != nullptr) // if we're not in global scope
                {
                    as.alloc_delta(*(scope->name), t_size);
                    var_symbol->stack_m = scope->stack_m -= t_size; // set stack location
                }
            }
            std::cout << "declared " << t << ' ' << identifier << " with definition " << std::endl;
            lcurrent = lcurrent->next; // and now, expression evaluation
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return eval_expression(lcurrent, var_symbol);
        }

        evaluation_state eval_variable_decl_def()
        {
            syntax_node* current = this->current;
            return eval_variable_decl_def(current);
        }

        evaluation_state eval_expression(syntax_node*& lcurrent, asc::symbol* application)
        {
            std::string location = "rax"; // ambiguous expression evaluations will be stored in rax
            if (application != nullptr) // if an application is provided, it will be stored at its stack location
                location = "[rbp + " + std::to_string(application->stack_m) + "]";
            std::string oper;
            for (int p_level = 0; !check_eof(lcurrent, true);)
            {
                if (*(lcurrent->value) == ")" && p_level > 0)
                {
                    p_level--;
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (*(lcurrent->value) == "(")
                {
                    p_level++;
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (eval_operator(lcurrent) == STATE_FOUND) // found an operator
                {
                    oper = *(lcurrent->value);
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (eval_exp_ending(lcurrent) == STATE_FOUND)
                {
                    lcurrent = lcurrent->next;
                    current = lcurrent;
                    return STATE_FOUND;
                }
                if (eval_numeric_literal(lcurrent) == STATE_FOUND) // found a number literal
                {
                    std::string word = application != nullptr ?
                            asc::get_word(asc::get_type_size(application->type)) + ' ' :
                            "";
                    if (oper == "+")
                        as.instruct(*(scope->name), "add " + word + location + ", " + *(lcurrent->value));
                    else if (oper == "-")
                        as.instruct(*(scope->name), "sub " + word + location + ", " + *(lcurrent->value));
                    else
                        as.instruct(*(scope->name), "mov " + word + location + ", " + *(lcurrent->value));
                    oper.clear(); // delete the operator
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (check_eof(lcurrent, true))
                    break;
                asc::symbol*& symbol = symbols[*(lcurrent->value)];
                if (symbol != nullptr)
                {
                    std::string symbol_loc = "[rbp + " + std::to_string(symbol->stack_m) + "]";
                    std::string res = asc::resolve_register(location, asc::get_type_size(symbol->type));
                    std::string word = asc::get_word(asc::get_type_size(symbol->type));
                    if (res != "-1")
                        location = res;
                    if (oper == "+")
                        as.instruct(*(scope->name), "add " + location + ", " + word + ' ' + symbol_loc);
                    else if (oper == "-")
                        as.instruct(*(scope->name), "sub " + location + ", " + word + ' ' + symbol_loc);
                    else
                        as.instruct(*(scope->name), "mov " + location + ", " + word + ' ' + symbol_loc);
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (check_eof(lcurrent, true))
                    break;
                if (application == nullptr)
                    as.instruct(*(scope->name), "push " + location); // preserve value in rax so a possible function call doesn't overwrite it
                if (eval_function_call(lcurrent) == STATE_FOUND)
                {
                    std::string word = application != nullptr ?
                            asc::get_word(asc::get_type_size(application->type)) + ' ' :
                            "";
                    if (application == nullptr)
                    {
                        as.instruct(*(scope->name), "mov rbx, rax"); // move function return value into rbx
                        as.instruct(*(scope->name), "pop " + location); // restore value
                    }
                    std::string ret_val_reg = application != nullptr ? "rax" : "rbx";
                    if (oper == "+")
                        as.instruct(*(scope->name), "add " + word + location + ", " + ret_val_reg);
                    else if (oper == "-")
                        as.instruct(*(scope->name), "sub " + word + location + ", " + ret_val_reg);
                    else
                        as.instruct(*(scope->name), "mov " + word + location + ", " + ret_val_reg);
                    oper.clear(); // delete the operator
                    lcurrent = lcurrent->next;
                    continue;
                }
                if (application == nullptr)
                    as.instruct(*(scope->name), "pop " + location); // preserve value in rax so a possible function call doesn't overwrite it
            }
            return STATE_NEUTRAL;
        }

        evaluation_state eval_expression()
        {
            syntax_node* current = this->current;
            return eval_expression(current, nullptr);
        }
    };
}

#endif