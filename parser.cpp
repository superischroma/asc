#include <stdexcept>
#include <algorithm>
#include <queue>
#include <stack>
#include <array>

#include "parser.h"

#define MAX_INT32 0x7FFFFFFF

namespace asc
{
    typedef struct
    {
        rpn_element* start;
        rpn_element* end;
    } function_argument_segment;

    symbol* invalid_symbol = nullptr;

    evaluation_state parser::recur_func_stack_args(syntax_node*& lcurrent, bool exp)
    {
        if (lcurrent == nullptr)
        {
            asc::err("unexpected end to argument passing");
            return STATE_SYNTAX_ERROR;
        }
        if (*(lcurrent->value) == ";") // finished
            return STATE_FOUND;
        evaluation_state es = recur_func_stack_args(lcurrent->next, *(lcurrent->value) == ",");
        if (es == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        if (!exp)
            return STATE_NEUTRAL;
        asc::debug("expression eval for recursive function arg definitions");
        evaluation_state ev_ex = eval_expression(lcurrent, nullptr); // eval for expression of current arg and store in rax
        if (ev_ex == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        if (ev_ex == STATE_NEUTRAL)
        {
            asc::err("expected expression", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        as.instruct(scope->name(), "push rax");
        return es;
    }

    parser::parser(syntax_node* root)
    {
        this->current = root;
        this->scope = nullptr;
        this->branchc = 0;
        this->slc = 0;
        this->dpc = 0;
        this->dpm = 0;
    }

    bool parser::parseable()
    {
        return current != nullptr;
    }

    bool parser::check_eof(syntax_node* node, bool silence)
    {
        if (node == nullptr)
        {
            if (!silence)
                asc::err("unexpected end of file");
            return true;
        }
        return false;
    }

    evaluation_state parser::eval_type(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        asc::debug("eval type for " + *(lcurrent->value));
        return asc::is_primitive(*(lcurrent->value));
    }

    evaluation_state parser::eval_type()
    {
        syntax_node* current = this->current;
        return eval_type(current);
    }

    evaluation_state parser::eval_visibility(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        asc::debug("eval visibility for " + *(lcurrent->value));
        return visibilities::value_of(asc::to_uppercase(*(lcurrent->value))) != visibilities::INVALID;
    }

    evaluation_state parser::eval_visibility()
    {
        syntax_node* current = this->current;
        return eval_visibility(current);
    }

    evaluation_state parser::eval_numeric_literal(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        return asc::is_numerical(*(lcurrent->value));
    }

    evaluation_state parser::eval_numeric_literal()
    {
        syntax_node* current = this->current;
        return eval_numeric_literal(current);
    }

    evaluation_state parser::eval_string_literal(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        return lcurrent->value->length() >= 2 && (*(lcurrent->value))[0] == '"' &&
            (*(lcurrent->value))[lcurrent->value->length() - 1] == '"';
    }

    evaluation_state parser::eval_exp_ending(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        std::string& value = *(lcurrent->value);
        return value == ";" || value == "," || value == ")";
    }

    evaluation_state parser::eval_exp_ending()
    {
        syntax_node* current = this->current;
        return eval_exp_ending(current);
    }

    evaluation_state parser::eval_operator(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        return asc::get_operator(*(lcurrent->value)) != -1;
    }

    evaluation_state parser::eval_operator()
    {
        syntax_node* current = this->current;
        return eval_operator(current);
    }

    evaluation_state parser::eval_function_decl(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        // errors will be thrown later on once we CONFIRM this is supposed to be a function declaration
        evaluation_state v_state = eval_visibility(lcurrent);
        std::string v = "private"; // default to private
        if (v_state == STATE_FOUND) // if a specifier was found, add it
            v = *(slcurrent->value);
        if (v_state == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        //asc::debug('v' << std::endl;
        if (v_state != STATE_NEUTRAL)
        {
            slcurrent = slcurrent->next;
            if (check_eof(slcurrent, true))
                return STATE_NEUTRAL;
        }
        int t_line = slcurrent->line;
        evaluation_state t_state = eval_type(slcurrent);
        if (t_state == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        std::string& t = *(slcurrent->value);
        int t_size = asc::get_type_size(t);
        //asc::debug('t' << std::endl;
        // at this point, it could still be a variable definition/declaration, so let's continue
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        int i_line = slcurrent->line;
        std::string& identifier = *(slcurrent->value); // get the identifier that MIGHT be there
        //asc::debug('i' << std::endl;
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        if (*(slcurrent->value) != "(") // if there is no parenthesis, it's confirmed that this is not a variable declaration
            return STATE_NEUTRAL; // return a neutral state, indicating no change
        lcurrent = slcurrent;
        // now let's throw some errors
        if (t_state == STATE_NEUTRAL) // if there was no type specifier, throw an error
        {
            asc::err("type specifier expected", t_line);
            return STATE_SYNTAX_ERROR;
        }
        if (symbol_table_get_imm(identifier) != nullptr)
        {
            asc::err("symbol is already defined");
            return STATE_SYNTAX_ERROR;
        }
        function_symbol* f_symbol = static_cast<function_symbol*>(symbol_table_insert(identifier, new asc::function_symbol(identifier, t,
            symbol_variants::FUNCTION, visibilities::value_of(asc::to_uppercase(v)), scope, 0)));
        //asc::debug("defined " << t << ' ' << identifier << " as a function" << std::endl;
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
            if (symbol_table_get_imm(a_identifier, f_symbol) != nullptr) // if symbol already exists in this scope
            {
                asc::err("symbol is already defined", ai_line);
                return STATE_SYNTAX_ERROR;
            }
            symbol* a_symbol = symbol_table_insert(a_identifier, new asc::symbol(a_identifier, at,
                symbol_variants::FUNCTION_VARIABLE, visibilities::PUBLIC, static_cast<symbol*>(f_symbol)));
            f_symbol->parameter_count++; // add parameter to count
            a_symbol->offset = s += 8;
            if (c <= 4)
                as.instruct(f_symbol->name(), "mov " + get_word(at_size) + " [rbp - " + std::to_string(a_symbol->offset) + "], " + asc::resolve_register(ARG_REGISTER_SEQUENCE[c - 1], at_size));
            //asc::debug("defined " << at << ' ' << a_identifier << " as a function argument for " << identifier << std::endl;
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
        scope = f_symbol; // scope into function
        lcurrent = lcurrent->next; // move into the function
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        current = lcurrent; // move member current to its proper location
        return STATE_FOUND; // finally, return the proper state
    }

    evaluation_state parser::eval_function_decl()
    {
        syntax_node* current = this->current;
        return eval_function_decl(current);
    }

    evaluation_state parser::eval_function_call(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        int i_line = slcurrent->line;
        std::string& identifier = *(slcurrent->value); // get the identifier that MIGHT be there
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        if (*(slcurrent->value) != "(") // if there is no parenthesis, it's confirmed that this is not a function call
            return STATE_NEUTRAL; // return a neutral state, indicating no change
        lcurrent = slcurrent;
        asc::symbol* function_symbol = symbol_table_get(identifier); // get reference to the current symbol with this identifier
        if (function_symbol == nullptr) // if symbol does not exist in this scope
        {
            asc::err("symbol is not defined", i_line);
            return STATE_SYNTAX_ERROR;
        }
        lcurrent = lcurrent->next;
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        std::string name = scope->name();
        int p_rcx = this->preserve_value("rcx");
        int p_rdx = this->preserve_value("rdx");
        int p_r8 = this->preserve_value("r8");
        int p_r9 = this->preserve_value("r9");
        // make sure none of these values
        // are modified during argument passing

        for (int i = 0; lcurrent != nullptr && eval_exp_ending(lcurrent) != STATE_FOUND && i < sizeof(ARG_REGISTER_SEQUENCE) / sizeof(ARG_REGISTER_SEQUENCE[0]); i++)
        {
            asc::debug("expression eval for argument variable");
            evaluation_state ev_ex = eval_expression(lcurrent, nullptr); // eval for expression of current arg and store in rax
            if (ev_ex == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (ev_ex == STATE_NEUTRAL)
            {
                asc::err("expected expression", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            as.instruct(scope->name(), "mov " + std::string(ARG_REGISTER_SEQUENCE[i]) + ", rax");
        }
        if (eval_exp_ending(lcurrent) != STATE_FOUND) // if there are still more arguments
        {
            if (recur_func_stack_args(lcurrent, true) == STATE_SYNTAX_ERROR) // iterate through stack args and push them
                return STATE_SYNTAX_ERROR;
        }
        as.instruct(scope->name(), "call " + identifier); // call the function

        this->retrieve_value(p_rcx, "rcx");
        this->retrieve_value(p_rdx, "rdx");
        this->retrieve_value(p_r8, "r8");
        this->retrieve_value(p_r9, "r9");

        if (lcurrent == nullptr) // unexpected end to arguments
        {
            asc::err("unexpected end to argument passing", i_line);
            return STATE_SYNTAX_ERROR;
        }
        //asc::debug("from: " << *(lcurrent->value) << std::endl;
        lcurrent = lcurrent->next; // move past ending parenthesis
        current = lcurrent;
        asc::debug("end with: " + *(lcurrent->value));
        return STATE_FOUND;
    }

    evaluation_state parser::eval_function_call()
    {
        syntax_node* current = this->current;
        return eval_function_call(current);
    }

    evaluation_state parser::eval_ret(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != "return") // not a return statement
            return STATE_NEUTRAL;
        asc::debug("expression eval for return expression");
        return eval_expression(lcurrent = lcurrent->next, nullptr);
    }

    evaluation_state parser::eval_ret()
    {
        syntax_node* current = this->current;
        return eval_ret(current);
    }

    evaluation_state parser::eval_if(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != "if") // not an if statement
            return STATE_NEUTRAL;
        lcurrent = lcurrent->next;
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        if (*(lcurrent->value) != "(")
        {
            asc::err("expected left parenthesis to start if statement");
            return STATE_SYNTAX_ERROR;
        }
        evaluation_state ev_ex = eval_expression(lcurrent = lcurrent->next, nullptr);
        if (ev_ex == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        if (ev_ex == STATE_NEUTRAL) // if there was no expression found
        {
            asc::err("expression expected");
            return STATE_SYNTAX_ERROR;
        }
        lcurrent = lcurrent->next; // move past left parenthesis
        //// TODO: ALLOW FOR ONE LINE IF STATEMENTS
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        lcurrent = lcurrent->next; // move past left brace, and into the if statement
        //// END TODO
        std::string ifbname = 'B' + std::to_string(++this->branchc); // if branch name
        std::string aftername = 'B' + std::to_string(++this->branchc); // after the if statement, plus split the current label
        as.instruct(scope->name(), "cmp rax, 0");       // if expression is not false
        as.instruct(scope->name(), "jne " + ifbname);   // jump to if branch
        as.instruct(scope->name(), "jmp " + aftername); // otherwise, jump to after branch
        std::string sname = scope->name();
        asc::subroutine*& csr = as.sr(sname); // current subroutine
        this->scope->split_b = this->branchc; // split the function by the after branch
        csr->ending = ""; // get rid of the ending for our current sr, because it will never be reached
        asc::subroutine*& ifb = as.sr(ifbname, csr); // if block subroutine
        asc::subroutine*& aftb = as.sr(aftername, csr); // after if block subroutine
        ifb->ending = "jmp " + aftername; // setting ending of if block to be the jump to the after block
        this->scope = new asc::symbol(ifbname, "if", symbol_variants::IF_BLOCK,
            visibilities::LOCAL, this->scope); // move scope into if statement
        current = lcurrent; // bring current up to speed
        return STATE_FOUND;
    }

    evaluation_state parser::eval_if()
    {
        asc::syntax_node* current = this->current;
        return eval_if(current);
    }

    evaluation_state parser::eval_while(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != "while") // not a while loop
            return STATE_NEUTRAL;
        lcurrent = lcurrent->next;
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        if (*(lcurrent->value) != "(")
        {
            asc::err("expected left parenthesis to start while condition");
            return STATE_SYNTAX_ERROR;
        }
        syntax_node* expression = (lcurrent = lcurrent->next);
        evaluation_state ev_ex = eval_expression(lcurrent, nullptr);
        if (ev_ex == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        if (ev_ex == STATE_NEUTRAL) // if there was no expression found
        {
            asc::err("expression expected");
            return STATE_SYNTAX_ERROR;
        }
        lcurrent = lcurrent->next; // move past left parenthesis
        //// TODO: ALLOW FOR ONE LINE WHILE LOOPS
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        lcurrent = lcurrent->next; // move past left brace, and into the if statement
        //// END TODO
        std::string loopbname = 'B' + std::to_string(++this->branchc); // if branch name
        std::string aftername = 'B' + std::to_string(++this->branchc); // after the if statement, plus split the current label
        as.instruct(scope->name(), "cmp rax, 0");       // if expression is not false
        as.instruct(scope->name(), "jne " + loopbname);   // jump to if branch
        as.instruct(scope->name(), "jmp " + aftername); // otherwise, jump to after branch
        std::string sname = scope->name();
        asc::subroutine*& csr = as.sr(sname); // current subroutine
        this->scope->split_b = this->branchc; // split the function by the after branch
        csr->ending = ""; // get rid of the ending for our current sr, because it will never be reached
        asc::subroutine*& loopb = as.sr(loopbname, csr); // if block subroutine
        loopb->ending = ""; // no ending
        asc::subroutine*& aftb = as.sr(aftername, csr); // after if block subroutine
        this->scope = new asc::symbol(loopbname, "while", symbol_variants::WHILE_BLOCK,
            visibilities::LOCAL, this->scope); // move scope into while loop
        this->scope->helper = expression; // preserve location of expression to be evaluated later
        current = lcurrent; // bring current up to speed
        return STATE_FOUND;
    }

    evaluation_state parser::eval_while()
    {
        asc::syntax_node* current = this->current;
        return eval_while(current);
    }

    evaluation_state parser::eval_block_ending(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != "}") // not a function ending
            return STATE_NEUTRAL;
        if (scope == nullptr) // if we're in the global scope
        {
            asc::err("attempting to scope out of the global scope", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        for (auto it = symbols.begin(); it != symbols.end();)
        // destroy all symbols in the current scope
        {
            auto& svp = *it;
            svp.second.erase(std::remove_if(svp.second.begin(), svp.second.end(), [this](symbol* s)
            {
                if (s->scope == scope)
                {
                    asc::debug("scope out: deleted symbol " + s->name());
                    return true;
                }
                return false;
            }), svp.second.end());
            if (svp.second.size() == 0)
            {
                symbols.erase(it++); // free up the memory from the vector
                continue;
            }
            ++it;
        }
        if (scope->variant == symbol_variants::FUNCTION) // if we're scoping out of a function
        {
            asc::debug("updating preserved for " + scope->m_name + ": " + std::to_string(dpm));
            as.sr(scope->m_name)->preserved_data = dpm; // set the max
            this->dpc = this->dpm = 0; // reset the dpc and dpm to be used later
        }
        if (scope->variant == symbol_variants::WHILE_BLOCK)
        {
            syntax_node*& cpy = scope->helper;
            asc::evaluation_state es_ev = eval_expression(cpy, nullptr);
            if (es_ev != asc::STATE_FOUND) // how the hell...
            {
                asc::err("you are impressively bad at programming...", cpy->line);
                return asc::STATE_SYNTAX_ERROR;
            }
            as.instruct(scope->name(), "cmp rax, 0");
            as.instruct(scope->name(), "jne " + scope->name());
            as.instruct(scope->name(), "jmp B" + std::to_string(std::stoi(scope->name().substr(1)) + 1));
        }
        if (scope->scope == nullptr)
            asc::debug("scoping out of " + scope->m_name + " into global scope");
        else
            asc::debug("scoping out of " + scope->m_name + " into " + scope->scope->m_name);
        scope = scope->scope; // scope out of function
        lcurrent = lcurrent->next;
        current = lcurrent;
        return STATE_FOUND;
    }

    evaluation_state parser::eval_block_ending()
    {
        syntax_node* current = this->current;
        return eval_block_ending(current);
    }

    evaluation_state parser::eval_variable_decl_def(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        // errors will be thrown later on once we CONFIRM this is supposed to be a function declaration
        evaluation_state v_state = eval_visibility(slcurrent);
        asc::debug("vis state: " + std::to_string((int) v_state));
        std::string v = "private"; // default to public
        if (v_state == STATE_FOUND) // if a specifier was found, add it
            v = *(slcurrent->value);
        if (v_state == STATE_SYNTAX_ERROR)
            return STATE_NEUTRAL;
        if (v_state != STATE_NEUTRAL)
        {
            slcurrent = slcurrent->next;
            if (check_eof(slcurrent, true))
                return STATE_NEUTRAL;
        }
        int t_line = slcurrent->line;
        evaluation_state t_state = eval_type(slcurrent);
        asc::debug("type state: " + std::to_string((int) t_state));
        if (t_state == STATE_SYNTAX_ERROR)
            return STATE_NEUTRAL;
        std::string& t = *(slcurrent->value);
        int t_size = asc::get_type_size(t);
        if (t_state != STATE_NEUTRAL)
        {
            slcurrent = slcurrent->next;
            if (check_eof(slcurrent, true))
                return STATE_NEUTRAL;
        }
        int i_line = slcurrent->line;
        std::string& identifier = *(slcurrent->value); // get the identifier that MIGHT be there
        //asc::debug("var decl def: " << v << ' ' << t << ' ' << identifier << std::endl;
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        if (*(slcurrent->value) == ";") // declaration or doing nothing with it (why)
        {
            lcurrent = slcurrent;
            if (t_state == STATE_NEUTRAL) // why...
            {
                if (symbol_table_get(identifier) == nullptr) // IT DOESN'T EVEN EXIST LOL
                {
                    asc::err("symbol is not defined", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
                lcurrent = lcurrent->next; // move it along after the semicolon
                current = lcurrent; // and bring current up to speed
                return STATE_FOUND;
            }
            asc::symbol* var_symbol = symbol_table_insert(identifier,
                new asc::symbol(identifier, t, symbol_variants::LOCAL_VARIABLE, visibilities::LOCAL, scope)); // define symbol
            if (scope == nullptr) // if we're in global scope
            {
                asc::err("constant must be initialized", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            var_symbol->offset = this->reserve_data_space(get_type_size(t)); // set stack location
            //asc::debug("declared " << t << ' ' << identifier << " without definition" << std::endl;
            lcurrent = lcurrent->next; // move it along after the semicolon
            current = lcurrent; // and bring current up to speed
            return STATE_FOUND;
        }
        if (*(slcurrent->value) != "=") // confirming it is a variable assignment
            return STATE_NEUTRAL;
        lcurrent = slcurrent;
        asc::symbol* var_symbol = symbol_table_get(identifier);
        asc::debug("type state: " + std::to_string(t_state));
        if (t_state == STATE_NEUTRAL && var_symbol == nullptr) // if attempting reassignment, but symbol doesn't exist
        {
            asc::err("symbol is not defined", i_line);
            return STATE_SYNTAX_ERROR;
        }
        if (t_state != STATE_NEUTRAL)
        {
            var_symbol = symbol_table_insert(identifier, new asc::symbol(identifier, t,
                (scope != nullptr ? symbol_variants::LOCAL_VARIABLE : symbol_variants::GLOBAL_VARIABLE),
                (scope != nullptr ? visibilities::LOCAL : visibilities::value_of(asc::to_uppercase(v))), scope));
            if (scope != nullptr) // if we're not in global scope
                var_symbol->offset = this->reserve_data_space(get_type_size(t));
            else // if we ARE in global scope
            {
                asc::err("constant variables have not been implemented yet", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
        }
        //asc::debug("declared " << t << ' ' << identifier << " with definition " << std::endl;
        lcurrent = lcurrent->next; // and now, expression evaluation
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        asc::debug("expression eval for " + var_symbol->name() + " variable decl");
        return eval_expression(lcurrent, var_symbol);
    }

    evaluation_state parser::eval_variable_decl_def()
    {
        syntax_node* current = this->current;
        return eval_variable_decl_def(current);
    }

    evaluation_state parser::eval_use(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != "use") // not a use statement
            return STATE_NEUTRAL; // neutral state indicating no change
        lcurrent = lcurrent->next;
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (lcurrent->type == asc::syntax_types::KEYWORD) // eventual handling for native use statements
        {
            asc::err("unimplemented feature: native use statements", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        else if (lcurrent->type == asc::syntax_types::IDENTIFIER) // function declarations
        {
            symbol_table_insert(*(lcurrent->value), new asc::symbol(*(lcurrent->value),
                "int", symbol_variants::FUNCTION, visibilities::PUBLIC, this->scope));
            as.external(*(lcurrent->value));
        }
        else
        {
            std::string& path = asc::unwrap(*(lcurrent->value));
            if (asc::compile(path) == -1) // if compilation doesn't work for external module
            {
                asc::err("usage compilation of " + path + " failed", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
        }
        lcurrent = lcurrent->next; // skip to semicolon
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*(lcurrent->value) != ";") // if this isn't a semicolon
        {
            asc::err("expected a semicolon", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        lcurrent = lcurrent->next; // go past semicolon
        current = lcurrent; // sync up current
        return STATE_FOUND; // return good state
    }

    evaluation_state parser::eval_use()
    {
        syntax_node* current = this->current;
        return eval_use(current);
    }

    evaluation_state parser::eval_expression(syntax_node*& lcurrent, asc::symbol* application)
    {
        std::string location = "rax"; // ambiguous expression evaluations will be stored in rax
        if (application != nullptr) // if an application is provided, it will be stored at its stack location
            location = "[rbp - " + std::to_string(application->offset) + "]";
        asc::debug("start of expression: " +
            (application != nullptr ? (application->name() + ", ") : "") +
            *(lcurrent->value) + ", " + location);
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
            //asc::debug("------" << std::endl;
            //asc::debug("expression: first: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_operator(lcurrent) == STATE_FOUND) // found an operator
            {
                asc::debug("expression: operator: " + *(lcurrent->value));
                oper = *(lcurrent->value);
                lcurrent = lcurrent->next;
                continue;
            }
            //asc::debug("expression: operator: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_exp_ending(lcurrent) == STATE_FOUND)
            {
                asc::debug("expression: exp ending: " + *(lcurrent->value));
                if (*(lcurrent->value) != ")") // if the ending is for a function call, don't move along so that the function call knows it has reached the end
                    lcurrent = lcurrent->next;
                current = lcurrent;
                return STATE_FOUND;
            }
            //asc::debug("expression: exp ending: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_numeric_literal(lcurrent) == STATE_FOUND) // found a number literal
            {
                ////asc::debug("expression: numeric literal: " << *(lcurrent->value) << std::endl;
                std::string word = application != nullptr ?
                        asc::get_word(asc::get_type_size(application->type)) + ' ' :
                        "";
                if (oper == "+")
                    as.instruct(scope->name(), "add " + word + location + ", " + *(lcurrent->value));
                else if (oper == "-")
                    as.instruct(scope->name(), "sub " + word + location + ", " + *(lcurrent->value));
                else
                    as.instruct(scope->name(), "mov " + word + location + ", " + *(lcurrent->value));
                oper.clear(); // delete the operator
                lcurrent = lcurrent->next;
                continue;
            }
            if (lcurrent->type == syntax_types::STRING_LITERAL)
            {
                std::string word = application != nullptr ?
                        asc::get_word(asc::get_type_size(application->type)) + ' ' :
                        "";
                std::string temp_name = "__strlit" + std::to_string(++slc);
                as << asc::data << temp_name + " db " + *(lcurrent->value) + ", 0x00";
                as.instruct(scope->name(), "mov " + word + location + ", " + temp_name);
                lcurrent = lcurrent->next;
                continue;
            }
            ////asc::debug("expression: numeric literal: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (check_eof(lcurrent, true))
                break;
            if (application == nullptr)
                preserve_value("rax"); // preserve value in rax so a possible function call doesn't overwrite it
            if (eval_function_call(lcurrent) == STATE_FOUND)
            {
                if (application != nullptr) // if an application is provided, it will be stored at its stack location
                    location = "[rbp - " + std::to_string(application->offset) + "]";
                asc::debug("function call validated from expression with an application of " +
                    (application != nullptr ? (application->name() + " and a location of ") : "") +
                    location);
                ////asc::debug("expression: function call: " << *(lcurrent->value) << std::endl;
                int a_size = 8;
                std::string word = application != nullptr ?
                        asc::get_word(a_size = asc::get_type_size(application->type)) + ' ' :
                        "";
                if (application == nullptr)
                {
                    as.instruct(scope->name(), "mov rbx, rax"); // move function return value into rbx
                    retrieve_value("rax"); // restore value
                }
                std::string ret_val_reg = resolve_register(application != nullptr ? "rax" : "rbx", a_size);
                if (oper == "+")
                    as.instruct(scope->name(), "add " + word + location + ", " + ret_val_reg);
                else if (oper == "-")
                    as.instruct(scope->name(), "sub " + word + location + ", " + ret_val_reg);
                else
                    as.instruct(scope->name(), "mov " + word + location + ", " + ret_val_reg);
                oper.clear(); // delete the operator
                asc::debug("ending? " + *(lcurrent->value));
                //if (eval_exp_ending(lcurrent) != STATE_FOUND) // if this is the end, keep it on the ending for later
                //    lcurrent = lcurrent->next; // otherwise, move on
                continue;
            }
            if (application == nullptr)
                retrieve_value("rax"); // preserve value in rax so a possible function call doesn't overwrite it
            //asc::debug("expression: function call: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            asc::symbol* symbol = symbol_table_get(*(lcurrent->value));
            if (symbol != nullptr)
            {
                std::string symbol_loc = "[rbp - " + std::to_string(symbol->offset) + "]";
                std::string res = asc::resolve_register(location, asc::get_type_size(symbol->type));
                asc::debug("lol: " + res);
                int symbol_size = asc::get_type_size(symbol->type);
                std::string word = asc::get_word(symbol_size);
                if (res != "-1")
                    location = res;
                std::string from = symbol_loc;
                if (application != nullptr) // attempt to move from memory location to memory location 
                {
                    // (not allowed in x86)
                    from = asc::resolve_register("rbx", symbol_size);
                    as.instruct(scope->name(), "mov " + // move value to auxilary register
                        from + ", " + word + ' ' + symbol_loc);
                }
                if (oper == "+")
                    as.instruct(scope->name(), "add " + location + ", " + word + ' ' + from);
                else if (oper == "-")
                    as.instruct(scope->name(), "sub " + location + ", " + word + ' ' + from);
                else
                    as.instruct(scope->name(), "mov " + location + ", " + word + ' ' + from);
                oper.clear();
                lcurrent = lcurrent->next;
                continue;
            }
            //asc::debug("expression: symbol: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (check_eof(lcurrent, true))
                break;
        }
        return STATE_NEUTRAL;
    }

    evaluation_state parser::eval_expression()
    {
        syntax_node* current = this->current;
        return eval_expression(current, nullptr);
    }

    evaluation_state parser::eval_var_declaration(syntax_node*& lcurrent)
    {
        syntax_node* slcurrent = lcurrent;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        visibility v = scope != nullptr ? visibilities::LOCAL : visibilities::value_of(to_uppercase(*(slcurrent->value)));
        if (v != visibilities::INVALID && v != visibilities::LOCAL)
            slcurrent = slcurrent->next;
        if (v == visibilities::INVALID)
            v = visibilities::PRIVATE;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        if (!is_type(*(slcurrent->value)))
            return STATE_NEUTRAL;
        std::string t = *(slcurrent->value);
        if (check_eof(slcurrent = slcurrent->next, true))
            return STATE_NEUTRAL;
        syntax_node* i_node = slcurrent; // copy identifier syntax node
        std::string i = *(slcurrent->value);
        if (symbol_table_get_imm(i) != nullptr) // symbol with this name already exists in this scope
        {
            asc::err("symbol is already defined", slcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        if (check_eof(slcurrent = slcurrent->next, true))
            return STATE_NEUTRAL;
        if (*(slcurrent->value) != "=" && *(slcurrent->value) != ";") // this is NOT a variable declaration (most likely a function declaration)
            return STATE_NEUTRAL;
        lcurrent = i_node; // sync up local with identifier node
        symbol* sym = symbol_table_insert(*(i_node->value), new asc::symbol(*(i_node->value), t,
            (scope != nullptr ? symbol_variants::LOCAL_VARIABLE : symbol_variants::GLOBAL_VARIABLE), v, scope));
        if (scope != nullptr)
            sym->offset = this->reserve_data_space(get_type_size(t));
        return experimental_eval_expression(i_node);
    }

    evaluation_state parser::eval_var_declaration()
    {
        syntax_node* current = this->current;
        return eval_var_declaration(current);
    }

    void parser::expression_instruct(std::string&& subroutine, std::string instruction,
        std::map<std::string, std::vector<std::queue<std::string>>>& fam, function_symbol* func, int index)
    {
        if (func != nullptr)
        {
            auto& list = fam[func->m_name];
            if (index >= list.size()) list.resize(index + 1);
            list[index].push(instruction);
        }
        else
            as.instruct(scope->name(), instruction);
    }

    evaluation_state parser::experimental_eval_expression(syntax_node*& lcurrent)
    {
        std::deque<rpn_element> output;
        std::stack<expression_operator> operators;

        std::stack<function_symbol*> functions;
        std::stack<int> call_indices;

        // shunting-yard algorithm: https://en.wikipedia.org/wiki/Shunting-yard_algorithm
        for (syntax_node* previous_node = nullptr; *lcurrent != ";"; previous_node = lcurrent, lcurrent = lcurrent->next)
        {
            if (lcurrent == nullptr)
            {
                if (!output.empty() || !operators.empty())
                {
                    asc::err("unexpected end of expression");
                    return STATE_SYNTAX_ERROR;
                }
                return STATE_NEUTRAL;
            }
            std::string& value = *(lcurrent->value);
            if (is_numerical(value))
            {
                output.push_back({ value, nullptr, !call_indices.empty() ? call_indices.top() : -1,
                    !functions.empty() ? functions.top() : nullptr });
            }
            else if (OPERATORS.count(value))
            {
                expression_operator oper = OPERATORS[value];

                int operands = 2;
                char c_fix = 'i';

                // manual prefix check
                if (previous_node == nullptr || *previous_node == "(" || OPERATORS.count(*(previous_node->value)))
                {
                    c_fix = 'p';
                    operands = 1;
                }

                // manual suffix check
                if (lcurrent->next == nullptr || *(lcurrent->next) == ")" || OPERATORS.count(*(lcurrent->next->value)))
                {
                    c_fix = 's';
                    operands = 1;
                }

                if (OPERATORS.count(value + std::to_string(operands) + c_fix))
                    oper = OPERATORS[value + std::to_string(operands) + c_fix];

                while (!operators.empty() && operators.top().value != "(" && (operators.top().precedence > oper.precedence ||
                    (operators.top().precedence == oper.precedence && oper.association)))
                {
                    output.push_back({ operators.top().value, &oper,
                        !call_indices.empty() ? call_indices.top() : -1,
                        !functions.empty() ? functions.top() : nullptr });
                    operators.pop();
                }

                if (oper.value == "," && !call_indices.empty()) // if it's a comma
                    call_indices.top()++; // add one to the index

                if (!oper.helper)
                    operators.push(oper);
            }
            else if (lcurrent->next != nullptr && *(lcurrent->next) == "(")
            {
                call_indices.push(0);
                functions.push(dynamic_cast<function_symbol*>(symbol_table_get(*(lcurrent->value))));
                operators.push({ *(lcurrent->value), 0, 2, LEFT_OPERATOR_ASSOCATION, INFIX_OPERATOR, false, true });
            }
            else if (*(lcurrent) == "(")
                operators.push({ *(lcurrent->value), 0, 2, LEFT_OPERATOR_ASSOCATION, INFIX_OPERATOR });
            else if (*(lcurrent) == ")")
            {
                while (true)
                {
                    if (operators.empty())
                    {
                        asc::err("closing parenthesis with no opening", lcurrent->line);
                        return STATE_SYNTAX_ERROR;
                    }
                    if (operators.top().value == "(") break;
                    output.push_back({ operators.top().value, nullptr,
                        !call_indices.empty() ? call_indices.top() : -1,
                        !functions.empty() ? functions.top() : nullptr });
                    operators.pop();
                }
                if (operators.empty() || operators.top().value != "(")
                {
                    asc::err("opening parenthesis expected", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
                operators.pop();
                if (!operators.empty() && operators.top().function)
                {
                    call_indices.pop();
                    functions.pop();
                    output.push_back({ operators.top().value, nullptr,
                        !call_indices.empty() ? call_indices.top() : -1,
                        !functions.empty() ? functions.top() : nullptr });
                    operators.pop();
                }
            }
            else
            {
                output.push_back({ *(lcurrent->value), nullptr,
                    !call_indices.empty() ? call_indices.top() : -1,
                    !functions.empty() ? functions.top() : nullptr });
            }
        }

        while (!operators.empty())
        {
            if (operators.top().value == "(")
            {
                asc::err("left parenthesis invalid");
                return STATE_SYNTAX_ERROR;
            }
            output.push_back({ operators.top().value, nullptr,
                !call_indices.empty() ? call_indices.top() : -1,
                !functions.empty() ? functions.top() : nullptr });
            operators.pop();
        }
        
        lcurrent = lcurrent->next; // skip over the semicolon which denoted the end of the expression
        current = lcurrent; // sync up our local current with the object member

        // christ almighty pt. 2
        std::unique_ptr<std::map<std::string, std::vector<std::queue<std::string>>>> f_arg_map =
            std::make_unique<std::map<std::string, std::vector<std::queue<std::string>>>>();
        // this is mapping function names to arguments which are lists of instruction queues

        int stack_uses = 0;

        // iterate over the reverse polish notation form of the expression which was parsed
        for (auto it = output.begin(); it != output.end(); it++)
        {
            asc::debug("expression evaluation state: " + stringify(output));
            rpn_element* element = &*it;
            std::string* token = &(element->value);
            asc::debug(" ^ currently evaluating for: " + *token);
            if (element->function)
                asc::debug(" ^ evaluation function: " + element->function->m_name);
            symbol* sym = symbol_table_get(*token);
            if (OPERATORS.count(*token))
            {
                auto& oper = OPERATORS[*token];
                std::array<rpn_element, 3> operands;
                int append_index = -1;
                if (it - oper.operands < output.begin())
                {
                    asc::err("operator expected " + std::to_string(oper.operands) + " operand" + (oper.operands != 1 ? "s" : ""));
                    return STATE_SYNTAX_ERROR;
                }
                for (int i = 0; i < oper.operands; i++)
                {
                    operands[i] = *(it - (oper.operands - i)); // extract operand
                    asc::debug("extracted operand for operator " + *token + ": " + operands[i].value);
                    it = output.erase(it - (oper.operands - i)) + (oper.operands - i - 1); // delete operand from deque
                    asc::debug("iterator put back at " + it->value);
                    if (operands[i].value == "..") // if it's the append indicator
                    {
                        append_index = i; // identify the index
                        continue; // skip
                    }
                    symbol* op_symbol = symbol_table_get(operands[i].value);
                    if (op_symbol == nullptr) // if the current operand isn't a symbol
                    {
                        if (!is_numerical(operands[i].value)) // if it's also not a numerical value
                        {
                            asc::err("symbol is not defined"); // throw an error
                            return STATE_SYNTAX_ERROR;
                        }
                    }
                    else
                    {
                        operands[i] = { "[rbp - " + std::to_string(op_symbol->offset) + ']',
                            operands[i].operator_data, operands[i].parameter_index };
                        // set the operand value as its location in the program
                    }
                }
                element = &*it;             // refresh these two values because
                token = &(element->value);  // of iterator invalidation
                std::string destination = "";
                if (element->parameter_index >= 0 && element->parameter_index <= 3)
                    destination = ARG_REGISTER_SEQUENCE[element->parameter_index];
                if (element->parameter_index > 3)
                    destination = "stack"; // empty string indicating it should go to stack
                asc::debug("destination set for " + *token + " operation: " + destination);
                bool specific_destination = !destination.empty();
                bool stack_destination = destination == "stack";
                bool first = append_index == -1; // is this operation the first part of this expression?
                if (oper.value == "+")
                {
                    if (first) // if it's first 
                    {
                        expression_instruct(scope->name(), "mov rax, " + operands[0].value,
                            *f_arg_map, element->function, element->parameter_index); // move in the first operand
                        expression_instruct(scope->name(), "add rax, " + operands[1].value,
                            *f_arg_map, element->function, element->parameter_index); // add the second operand
                    }
                    else
                    {
                        expression_instruct(scope->name(), "add rax, " + operands[!append_index].value,
                            *f_arg_map, element->function, element->parameter_index); // no special things because of communative property of addition
                    }
                }
                else if (oper.value == "=")
                {
                    if (first)
                    {
                        expression_instruct(scope->name(), "mov rax, " + operands[1].value,
                            *f_arg_map, element->function, element->parameter_index);
                        expression_instruct(scope->name(), "mov " + operands[0].value + ", rax",
                            *f_arg_map, element->function, element->parameter_index);
                    }
                    else
                    {
                        expression_instruct(scope->name(), "mov " + operands[!append_index].value + ", rax",
                            *f_arg_map, element->function, element->parameter_index);
                    }
                }
                else
                {
                    asc::err("unimplemented operator used");
                    return STATE_SYNTAX_ERROR;
                }
                if (specific_destination)
                {
                    if (stack_destination)
                    {
                        expression_instruct(scope->name(), "push rax",
                            *f_arg_map, element->function, element->parameter_index);
                    }
                    else
                    {
                        expression_instruct(scope->name(), std::string("mov ") +
                            ARG_REGISTER_SEQUENCE[element->parameter_index] + ", rax", *f_arg_map,
                            element->function, element->parameter_index);
                    }
                }
                asc::debug("element value for " + *token + " has been set to expander");
                element->value = "..";
            }
            else if (sym != nullptr && sym->variant == symbol_variants::FUNCTION)
            {
                function_symbol* f_sym = static_cast<function_symbol*>(sym);
                auto& q = (*f_arg_map)[f_sym->m_name];
                for (auto vit = q.end() - 1; vit >= q.begin(); vit--)
                {
                    for (; !(vit->empty()); vit->pop())
                    {
                        std::cout << "add: " << vit->front() << std::endl;
                        as.instruct(scope->name(), vit->front());
                    }
                }
                f_arg_map->erase(f_sym->m_name); // free up the room held by the vector
                as.instruct(scope->name(), "call " + f_sym->m_name);
                element = &*it;             // refresh these two values because
                token = &(element->value);  // of iterator invalidation
                element->value = "..";
            }
        }

        return STATE_FOUND;
    }

    evaluation_state parser::experimental_eval_expression()
    {
        syntax_node* current = this->current;
        return experimental_eval_expression(current);
    }

    evaluation_state parser::experimental_eval_return_statement(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*lcurrent != "return")
            return STATE_NEUTRAL;
        return experimental_eval_expression(lcurrent = lcurrent->next);
    }

    evaluation_state parser::experimental_eval_return_statement()
    {
        syntax_node* current = this->current;
        return experimental_eval_return_statement(current);
    }

    evaluation_state parser::eval_type_construct(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        evaluation_state v_state = eval_visibility(lcurrent);
        std::string v = "private"; // default to private
        if (v_state == STATE_FOUND) // if a specifier was found, add it
            v = *(slcurrent->value);
        if (v_state == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        if (check_eof(slcurrent = slcurrent->next))
            return STATE_NEUTRAL;
        if (*(slcurrent) != "type") // not a type
            return STATE_NEUTRAL;
        lcurrent = slcurrent; // sync up local current with super local current
        if (check_eof(lcurrent = lcurrent->next)) // move forward to identifier
            return STATE_SYNTAX_ERROR;
        std::string identifier = *(lcurrent->value); // get the identifier
        if (check_eof(lcurrent = lcurrent->next)) // move forward
            return STATE_SYNTAX_ERROR;
        if ((*lcurrent) == "extends")
        {
            asc::err("inheritance is not implemented yet", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        if ((*lcurrent) != "{") // if we're not starting the type
        {
            asc::err("type definition expected", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        if (check_eof(lcurrent = lcurrent->next)) // move past brace
            return STATE_SYNTAX_ERROR;
        type_symbol* sym = new type_symbol(identifier, "type", symbol_variants::STRUCTLIKE_TYPE,
            visibilities::value_of(asc::to_uppercase(v)), this->scope);
        symbol_table_insert(identifier, sym);
        while (!check_eof(lcurrent) && *(lcurrent) != "}")
        {
            int t_line = lcurrent->line;
            evaluation_state t_state = eval_type(lcurrent);
            if (t_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (t_state == STATE_NEUTRAL)
            {
                asc::err("type expected", t_line);
                return STATE_SYNTAX_ERROR;
            }
            std::string& t = *(lcurrent->value);
            int t_size = asc::get_type_size(t);
            if (check_eof(lcurrent = lcurrent->next)) // move to identifier
                return STATE_SYNTAX_ERROR;
            std::string identifier = *(lcurrent->value);
            if (lcurrent->type != syntax_types::IDENTIFIER)
            {
                asc::err("identifier expected", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            syntax_node* identifier_node = lcurrent;
            sym->members.push_back(identifier_node);
            symbol_table_insert(identifier, new symbol(identifier, t, symbol_variants::STRUCTLIKE_TYPE_MEMBER,
                visibilities::PUBLIC, static_cast<symbol*>(sym)));
            while (!check_eof(lcurrent = lcurrent->next) && *(lcurrent) != ";");
            if (lcurrent == nullptr)
                return STATE_SYNTAX_ERROR;
            if (check_eof(lcurrent = lcurrent->next)) // skip semicolon
                return STATE_SYNTAX_ERROR;
        }
        current = lcurrent = lcurrent->next;
        return STATE_FOUND;
    }

    evaluation_state parser::eval_type_construct()
    {
        syntax_node* current = this->current;
        return eval_type_construct(current);
    }

    int parser::preserve_value(std::string location, symbol* scope)
    {
        int register_size = asc::get_register_size(location);
        int position = (dpc += register_size);
        if (dpc > dpm) dpm = dpc; // update max if needed
        as.instruct(scope != nullptr ? scope->name() : this->scope->name(), "mov [rbp - " + std::to_string(position) + "], " + location);
        return position;
    }

    int parser::reserve_data_space(int size)
    {
        int position = (dpc += size);
        if (dpc > dpm) dpm = dpc; // update max if needed
        return position;
    }

    void parser::retrieve_value(int position, std::string storage)
    {
        as.instruct(scope->name(), "mov " + storage + ", [rbp - " + std::to_string(position) + ']');
        dpc -= asc::get_register_size(storage);
    }

    // off the top
    void parser::retrieve_value(std::string storage)
    {
        retrieve_value(dpc, storage);
    }

    bool parser::symbol_table_has(std::string name, symbol* scope)
    {
        try
        {
            symbol_table_get(name, scope);
            return true;
        }
        catch (std::out_of_range e)
        {
            return false;
        }
    }

    /**
     * @brief Retrieve a symbol from the symbol table. This method
     * will return symbols in lower scopes if it does not find one
     * of the name in the scope provided.
     * 
     * @param name The symbol's name
     * @param scope The scope to search from
     * @return symbol*& 
     */
    symbol* parser::symbol_table_get(std::string name, symbol* scope)
    {
        if (scope == nullptr)
            scope = this->scope;
        std::vector<symbol*>* found; // get symbols with this name
        try
        {
            found = &(symbols.at(name));
        }
        catch (std::out_of_range e)
        {
            return nullptr;
        }
        if (found->size() == 1) // if there's only one
            return (*found)[0]; // return it
        int priority = -1; // keep track of the best instance of this symbol's index
        int depth = MAX_INT32; // keep track of the depth of the best instance
        for (int i = 0; i < found->size(); i++) // iterate over the symbols with this name
        {
            int d = 0;
            for (symbol* s = scope; s != nullptr; s = s->scope, d++) // iterate down ideal scope
            {
                if (s->name() == (*found)[i]->scope->name())
                {
                    if (d < depth)
                    {
                        priority = i;
                        depth = d;
                    }
                    break;
                }
            }
        }
        if (priority == -1)
            return nullptr;
        return (*found)[priority];
    }

    /**
     * @brief Retrieve a symbol from the symbol table in
     * the immediate scope without checking lower ones.
     * 
     * @param name The symbol's name
     * @param scope The scope to search in
     * @return symbol*& 
     */
    symbol* parser::symbol_table_get_imm(std::string name, symbol* scope)
    {
        if (scope == nullptr)
            scope = this->scope;
        std::vector<symbol*>* found; // get symbols with this name
        try
        {
            found = &(symbols.at(name));
        }
        catch (std::out_of_range e)
        {
            return nullptr;
        }
        for (int i = 0; i < found->size(); i++)
        {
            if (scope == (*found)[i]->scope)
                return (*found)[i];
        }
        return nullptr;
    }

    symbol* parser::symbol_table_insert(std::string name, symbol* s)
    {
        symbols[name].push_back(s);
        return s;
    }

    void parser::symbol_table_delete(symbol* s)
    {
        std::vector<symbol*>& vec = symbols[s->m_name];
        vec.erase(std::remove(vec.begin(), vec.end(), s), vec.end());
        if (vec.empty())
            symbols.erase(s->m_name); // free some memory if we're not using the vector
    }

    bool parser::is_type(std::string str)
    {
        if (primitives::from_display(str) != primitives::INVALID)
            return true;
        symbol* sym = symbol_table_get(str);
        if (sym == nullptr)
            return false;
        return sym->variant == symbol_variants::STRUCTLIKE_TYPE || sym->variant == symbol_variants::OBJECT;
    }

    parser::~parser()
    {
        if (scope != nullptr)
            delete scope;
    }
}