#ifndef PARSER_H
#define PARSER_H

#include <map>

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
    public:
        // tracking variables
        syntax_node* current; // syntax token being evaluated
        assembler as; // constructor for assembly code
        std::map<std::string, symbol*> symbols; // symbol table
        symbol* scope; // scope of next tokens, null if global
        /*
        symbol* csymbol; // symbol being evaluated for, if any
        // options
        bool declaration; // whether the symbol evaluation is for declaration or another purpose (calling a function or reassigning a variable)
        char visibility; // the visibility for the symbol being evaluated (public, private, protected)
        char oper; // the current operator being worked on for an expression
        std::string type; // the type of the symbol being evaluated
        */
        parser(syntax_node* root)
        {
            this->current = root;
            /*
            this->scope = nullptr;
            this->csymbol = nullptr;
            this->declaration = false;
            this->visibility = -1;
            this->oper = -1;
            */
        }

        bool parseable()
        {
            return current != nullptr;
        }

        bool check_eof(syntax_node* node, bool silence = false)
        {
            if (node == nullptr && !silence)
            {
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
            return eval_numeric_literal(current);
        }

        evaluation_state eval_exp_ending(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            std::string& value = *(lcurrent->value);
            return value == ";" || value == ",";
        }

        evaluation_state eval_exp_ending()
        {
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
            if (v_state == STATE_NEUTRAL)
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
            // at this point, it could still be a variable definition/declaration, so let's continue
            lcurrent = lcurrent->next;
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            int i_line = lcurrent->line;
            std::string& identifier = *(lcurrent->value); // get the identifier that MIGHT be there
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
            for (int c = 1; true; c++) // loop until we're at the end of the declaration, this is an infinite loop to make code smoother
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
                a_symbol = new asc::symbol(a_identifier, at, "", function_symbol);
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
            while (lcurrent != nullptr && eval_exp_ending(lcurrent) != STATE_FOUND)
            {
                if (eval_expression(lcurrent) == STATE_SYNTAX_ERROR)
                    return STATE_SYNTAX_ERROR;
            }
            if (lcurrent == nullptr) // unexpected end to arguments
            {
                asc::err("unexpected end to argument passing", i_line);
                return STATE_SYNTAX_ERROR;
            }
            lcurrent = lcurrent->next; // move past semicolon
            current = lcurrent;
            return STATE_FOUND;
        }

        evaluation_state eval_function_call()
        {
            return eval_function_call(current);
        }

        evaluation_state eval_ret(syntax_node*& lcurrent)
        {
            if (check_eof(lcurrent, true))
                return STATE_NEUTRAL;
            if (*(lcurrent->value) != "return") // not a return statement
                return STATE_NEUTRAL;
            return eval_expression(lcurrent = lcurrent->next);
        }

        evaluation_state eval_ret()
        {
            return eval_ret(current);
        }

        evaluation_state eval_variable_decl_def(syntax_node*& lcurrent)
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
            if (v_state == STATE_NEUTRAL)
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
            if (t_state == STATE_NEUTRAL)
            {
                lcurrent = lcurrent->next;
                if (check_eof(lcurrent, true))
                    return STATE_NEUTRAL;
            }
            int i_line = lcurrent->line;
            std::string& identifier = *(lcurrent->value); // get the identifier that MIGHT be there
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
                        std::cout << v << ", " << t << ", " << identifier << std::endl;
                        asc::err("symbol is not defined", lcurrent->line);
                        return STATE_SYNTAX_ERROR;
                    }
                    lcurrent = lcurrent->next; // move it along after the semicolon
                    current = lcurrent; // and bring current up to speed
                    return STATE_FOUND;
                }
                var_symbol = new asc::symbol(identifier, t, v, scope); // define symbol
                lcurrent = lcurrent->next; // move it along after the semicolon
                current = lcurrent; // and bring current up to speed
                return STATE_FOUND;
            }
            if (*(lcurrent->value) != "=") // confirming it is NOT a variable assignment
                return STATE_NEUTRAL;
            asc::symbol*& var_symbol = symbols[identifier];
            if (t_state == STATE_NEUTRAL && var_symbol == nullptr) // if attempting reassignment, but symbol doesn't exist
            {
                asc::err("c");
                asc::err("symbol is not defined", i_line);
                return STATE_SYNTAX_ERROR;
            }
            lcurrent = lcurrent->next; // and now, expression evaluation
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            return eval_expression(lcurrent);
        }

        evaluation_state eval_variable_decl_def()
        {
            return eval_variable_decl_def(current);
        }

        evaluation_state eval_expression(syntax_node*& lcurrent)
        {
            for (int p_level = 0; !check_eof(lcurrent, true); lcurrent = lcurrent->next)
            {
                if (*(lcurrent->value) == ")" && p_level > 0)
                {
                    p_level--;
                    continue;
                }
                if (*(lcurrent->value) == "(")
                {
                    p_level++;
                    continue;
                }
                if (eval_exp_ending(lcurrent) == STATE_FOUND)
                {
                    lcurrent = lcurrent->next;
                    current = lcurrent;
                    return STATE_FOUND;
                }
                if (eval_numeric_literal(lcurrent) == STATE_FOUND)
                {
                    continue;
                }
                if (eval_function_call(lcurrent) == STATE_FOUND)
                {
                    continue;
                }
                asc::symbol*& symbol = symbols[*(lcurrent->value)];
                if (symbol != nullptr)
                {
                    continue;
                }
            }
            return STATE_NEUTRAL;
        }

        evaluation_state eval_expression()
        {
            return eval_expression(current);
        }

        /*
        bool parse_expression(symbol*& fsymbol)
        {
            for (; current != nullptr; current = current->next)
            {
                std::string& value = *(current->value);
                std::cout << value << ": " << (int) declaration << ", " << (int) oper << ", " << type << std::endl;
                if (value == ";" || value == "{" || value == "," || value == ")" || value.length() == 0) // if we reached the end of the expression
                {
                    current = current->next;
                    fsymbol = nullptr;
                    return true;
                }
                if (value == "}")
                {
                    if (!scope_out(fsymbol))
                        return false;
                    current = current->next;
                    fsymbol = nullptr;
                    return true;
                }
                if (value == "=")
                {
                    if (!parse_assignment(fsymbol))
                        return false;
                    continue;
                }
                char v = mark_visibility_specifier();
                if (v == 0)
                    return false;
                if (v == 2)
                    continue;
                char t = mark_type_specifier();
                if (t == 0)
                    return false;
                if (t == 2)
                    continue;
                if (mark_oper())
                    continue;
                if (value == "return")
                {
                    if (!parse_ret(fsymbol))
                        return false;
                    continue;
                }
                if (is_numerical(value)) // basic numerical checking for now
                {
                    if (!parse_constant(fsymbol))
                        return false;
                    continue;
                }
                if (value == "(" && fsymbol != nullptr) // marks function or function call
                {
                    if (!parse_function(fsymbol))
                        return false;
                    continue;
                }
                if (!eval_symbol(fsymbol))
                    return false;
            }
            if (current == nullptr)
            {
                asc::err("unexpected end of expression");
                return false;
            }
            fsymbol = nullptr;
            return true;
        }

        bool parse_expression()
        {
            return parse_expression(csymbol);
        }

        bool parse_assignment(symbol*& fsymbol)
        {
            return true;
        }

        bool parse_constant(symbol*& fsymbol)
        {
            std::string& value = *(current->value);
            if (scope == nullptr) // if we're in global scope
            {
                if (fsymbol == nullptr) // if we aren't parsing an expression for a symbol
                {
                    asc::err("stranded constant", current->line);
                    return false;
                }
                asc::warn("global constants not allowed yet");
            }
            else
            {
                std::cout << "parsing constant for " << fsymbol << std::endl;
                if (fsymbol != nullptr) // if this is not a standalone constant
                {
                    int size = get_type_size(fsymbol->type);
                    std::string word = get_word(size);
                    if (oper <= 0)
                    {
                        fsymbol->stack_m = scope->stack_m -= size; // store stack location for identifier and move down stack tracker for scope
                        std::string loc = std::to_string(fsymbol->stack_m);
                        if (fsymbol->stack_m >= 0)
                            loc = '+' + loc;
                        loc.insert(0, " ");
                        loc.insert(2, " ");
                        as.alloc_delta(*(scope->name), size);
                        std::cout << "declaration: " << *(scope->name) << ", " << *(fsymbol->name) << ", " << fsymbol->type << ", " << size << std::endl;
                        as.instruct(*(scope->name), "mov " + word + " [rbp" + loc + "], " + value);
                    }
                    else
                    {
                        std::string loc = std::to_string(fsymbol->stack_m);
                        if (fsymbol->stack_m >= 0)
                            loc = '+' + loc;
                        loc.insert(0, " ");
                        loc.insert(2, " ");
                        if (oper == 1) // +
                            as.instruct(*(scope->name), "add " + word + " [rbp" + loc + "], " + value);
                        if (oper == 2) // -
                            as.instruct(*(scope->name), "sub " + word + " [rbp" + loc + "], " + value);
                        oper = -1;
                    }
                }
                else
                {
                    if (oper <= 0)
                        as.instruct(*(scope->name), "mov eax, " + value);
                    else
                    {
                        if (oper == 1)
                            as.instruct(*(scope->name), "add eax, " + value);
                        if (oper == 2)
                            as.instruct(*(scope->name), "sub eax, " + value);
                        oper = -1;
                    }
                }
            }
            return true;
        }

        bool parse_function(symbol*& fsymbol)
        {
            if (current->next == nullptr) // incomplete function/function call
            {
                asc::err("incomplete statement", current->line);
                return false;
            }
            current = current->next;
            if (*(current->value) == ")" && !declaration) // if 0 args are present
            {
                as.instruct(*(scope->name), "call " + *(fsymbol->name)); // just call it
                current = current->next;
            }
            else
            {
                for (int i = 0; current != nullptr; current = current->next)
                {
                    if (*(current->value) == ")")
                        break;
                    if (*(current->value) == ",")
                        continue;
                    if (declaration)
                    {
                        bool prim = is_primitive(*(current->value));
                        if (!prim)
                        {
                            asc::err(*(current->value));
                            asc::err("symbol has not been defined", current->line);
                            return false;
                        }
                        std::string type = *(current->value);
                        if (current->next == nullptr)
                        {
                            asc::err("identifier expected", current->line);
                            return false;
                        }
                        int size = get_type_size(type);
                        syntax_node*& param = current = current->next;
                        symbol*& s = symbols[*(param->value)];
                        if (s != nullptr)
                        {
                            asc::err("symbol has already been defined", current->line);
                            return false;
                        }
                        s = new symbol(*(param->value), type, fsymbol);
                        if (i < 4)
                            as.instruct(*(fsymbol->name), "mov " + get_word(size) + " [rbp + " + std::to_string((i * 8) + 16) + "], " + resolve_register(ARG_REGISTER_SEQUENCE[i], size));
                        i++;
                    }
                    else
                    {
                        asc::warn("uncompleted action made: calling non-0-arg function");
                        parse_expression();
                    }
                }
                if (declaration)
                {
                    std::cout << "scoped into " << *(fsymbol->name) << std::endl;
                    scope = fsymbol; // move current scope into function
                }
            }
            return true;
        }

        bool parse_ret(symbol*& fsymbol)
        {
            if (current->next == nullptr)
            {
                asc::err("semicolon or expression expected", current->line);
                return false;
            }
            return true;
        }

        bool scope_out(symbol*& fsymbol)
        {
            if (scope == nullptr)
            {
                asc::err("attempt to scope out of global scope", current->line);
                return false;
            }
            scope = scope->scope;
            return true;
        }

        char mark_visibility_specifier()
        {
            char v_id = asc::get_visibility_id(*(current->value));
            if (v_id != -1 && visibility != -1)
            {
                asc::err("visibility for this symbol has already been specified", current->line);
                return 0;
            }
            if (v_id != -1)
            {
                visibility = v_id;
                return 2;
            }
            return 1;
        }

        char mark_type_specifier()
        {
            bool prim = asc::is_primitive(*(current->value));
            if (prim && type.length() != 0)
            {
                asc::err("type for this symbol has already been specified", current->line);
                return 0;
            }
            if (prim)
            {
                type = *(current->value);
                return 2;
            }
            return 1;
        }

        bool mark_oper()
        {
            char o_id = asc::get_operator_id(*(current->value));
            if (o_id != -1)
            {
                oper = o_id;
                return true;
            }
            return false;
        }

        bool eval_symbol(symbol*& fsymbol)
        {
            std::string& value = *(current->value);
            asc::symbol*& symbol = symbols[value];
            declaration = type.length() != 0;
            if (symbol != nullptr && declaration) // it's either being used or being redefined
            // check if a type has been specified for it (it's probably being redefined)
            {
                asc::err("symbol has already been defined", current->line);
                return false;
            }
            if (symbol == nullptr && !declaration)
            // check if no type has been specified and it's not defined
            {
                asc::err("symbol has not been defined", current->line);
                return false;
            }
            if (visibility == -1) // if no visibility was specified for this member
                visibility = 0; // make it public by default
            if (declaration)
            {
                std::cout << "defined symbol " << value << " as type " << type << std::endl;
                symbol = new asc::symbol(value, type, scope);
            }
            visibility = -1;
            type.erase();
            if (current->next == nullptr)
            {
                asc::err("semicolon or declaration expected", current->line);
                return false;
            }
            fsymbol = symbol;
            return true;
        }
        */
    };
}

#endif