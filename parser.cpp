#include "parser.h"

namespace asc
{
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
        std::cout << "expression eval for recursive function arg definitions" << std::endl;
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
        return asc::get_visibility_id(*(lcurrent->value)) != -1;
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
        std::string v = "private"; // default to public
        if (v_state == STATE_FOUND) // if a specifier was found, add it
            v = *(slcurrent->value);
        if (v_state == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
        //std::cout << 'v' << std::endl;
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
        //std::cout << 't' << std::endl;
        // at this point, it could still be a variable definition/declaration, so let's continue
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        int i_line = slcurrent->line;
        std::string& identifier = *(slcurrent->value); // get the identifier that MIGHT be there
        //std::cout << 'i' << std::endl;
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
        asc::symbol*& function_symbol = symbols[identifier]; // get reference to the current symbol with this identifier
        if (function_symbol != nullptr) // if symbol already exists in this scope
        {
            asc::err("symbol is already defined", i_line);
            return STATE_SYNTAX_ERROR;
        }
        function_symbol = new asc::symbol(identifier, t, symbol_types::FUNCTION, v, scope);
        //std::cout << "defined " << t << ' ' << identifier << " as a function" << std::endl;
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
            a_symbol = new asc::symbol(a_identifier, at, symbol_types::FUNCTION_VARIABLE, "public", function_symbol);
            a_symbol->offset = s += 8;
            if (c <= 4)
                as.instruct(function_symbol->name(), "mov " + get_word(at_size) + " [rbp - " + std::to_string(a_symbol->offset) + "], " + asc::resolve_register(ARG_REGISTER_SEQUENCE[c - 1], at_size));
            //std::cout << "defined " << at << ' ' << a_identifier << " as a function argument for " << identifier << std::endl;
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
        asc::symbol*& function_symbol = symbols[identifier]; // get reference to the current symbol with this identifier
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
            std::cout << "expression eval for argument variable" << std::endl;
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
        //std::cout << "from: " << *(lcurrent->value) << std::endl;
        lcurrent = lcurrent->next; // move past ending parenthesis
        current = lcurrent;
        std::cout << "end with: " << *(lcurrent->value) << std::endl;
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
        std::cout << "expression eval for return expression" << std::endl;
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
        this->scope = new asc::symbol(ifbname, "if", symbol_types::IF_BLOCK, "public", this->scope); // move scope into if statement
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
        this->scope = new asc::symbol(loopbname, "while", symbol_types::WHILE_BLOCK, "public", this->scope); // move scope into while loop
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
        for (auto it = symbols.cbegin(); it != symbols.cend();)
        // destroy all symbols in the current scope
        {
            if (it->second == nullptr)
                continue;
            if (it->second->scope == scope)
            {
                auto symbol = it->second;
                std::cout << "scope out: deleted symbol " << symbol->name() << std::endl;
                symbols.erase(it++);
                delete symbol;
            }
            else
                ++it;
        }
        if (scope->s_type == symbol_types::FUNCTION) // if we're scoping out of a function
        {
            std::cout << "updating preserved for " << scope->m_name << ": " << dpm << std::endl;
            as.sr(scope->m_name)->preserved_data = dpm; // set the max
            this->dpc = this->dpm = 0; // reset the dpc and dpm to be used later
        }
        if (scope->s_type == symbol_types::WHILE_BLOCK)
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
            std::cout << "scoping out of " << scope->m_name << " into global scope" << std::endl;
        else
            std::cout << "scoping out of " << scope->m_name << " into " << scope->scope->m_name << std::endl;
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
        //std::cout << "eval var dec def vis: " << *(slcurrent->value) << std::endl;
        std::string v = "private"; // default to public
        if (v_state == STATE_FOUND) // if a specifier was found, add it
            v = *(slcurrent->value);
        if (v_state == STATE_SYNTAX_ERROR)
            return STATE_SYNTAX_ERROR;
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
        if (t_state != STATE_NEUTRAL)
        {
            slcurrent = slcurrent->next;
            if (check_eof(slcurrent, true))
                return STATE_NEUTRAL;
        }
        int i_line = slcurrent->line;
        std::string& identifier = *(slcurrent->value); // get the identifier that MIGHT be there
        //std::cout << "var decl def: " << v << ' ' << t << ' ' << identifier << std::endl;
        slcurrent = slcurrent->next;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        if (*(slcurrent->value) == ";") // declaration or doing nothing with it (why)
        {
            lcurrent = slcurrent;
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
            var_symbol = new asc::symbol(identifier, t, symbol_types::LOCAL_VARIABLE, v, scope); // define symbol
            if (scope == nullptr) // if we're in global scope
            {
                asc::err("constant must be initialized", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            var_symbol->offset = this->reserve_data_space(get_type_size(t)); // set stack location
            //std::cout << "declared " << t << ' ' << identifier << " without definition" << std::endl;
            lcurrent = lcurrent->next; // move it along after the semicolon
            current = lcurrent; // and bring current up to speed
            return STATE_FOUND;
        }
        if (*(slcurrent->value) != "=") // confirming it is a variable assignment
            return STATE_NEUTRAL;
        lcurrent = slcurrent;
        asc::symbol*& var_symbol = symbols[identifier];
        if (t_state == STATE_NEUTRAL && var_symbol == nullptr) // if attempting reassignment, but symbol doesn't exist
        {
            asc::err("symbol is not defined", i_line);
            return STATE_SYNTAX_ERROR;
        }
        if (t_state != STATE_NEUTRAL)
        {
            var_symbol = new asc::symbol(identifier, t, (scope != nullptr ?
                symbol_types::LOCAL_VARIABLE : symbol_types::GLOBAL_VARIABLE), v, scope);
            if (scope != nullptr) // if we're not in global scope
                var_symbol->offset = this->reserve_data_space(get_type_size(t));
            else // if we ARE in global scope
            {
                asc::err("constant variables have not been implemented yet", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
        }
        //std::cout << "declared " << t << ' ' << identifier << " with definition " << std::endl;
        lcurrent = lcurrent->next; // and now, expression evaluation
        if (check_eof(lcurrent))
            return STATE_SYNTAX_ERROR;
        std::cout << "expression eval for " << var_symbol->name() << " variable decl" << std::endl;
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
            symbols[*(lcurrent->value)] = new asc::symbol(*(lcurrent->value),
                "", symbol_types::FUNCTION, "public", this->scope);
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
        std::cout << "start of expression: ";
        if (application != nullptr)
            std::cout << application->name() << ", ";
        std::cout << *(lcurrent->value) << ", " << location << std::endl;
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
            //std::cout << "------" << std::endl;
            //std::cout << "expression: first: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_operator(lcurrent) == STATE_FOUND) // found an operator
            {
                std::cout << "expression: operator: " << *(lcurrent->value) << std::endl;
                oper = *(lcurrent->value);
                lcurrent = lcurrent->next;
                continue;
            }
            //std::cout << "expression: operator: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_exp_ending(lcurrent) == STATE_FOUND)
            {
                std::cout << "expression: exp ending: " << *(lcurrent->value) << std::endl;
                if (*(lcurrent->value) != ")") // if the ending is for a function call, don't move along so that the function call knows it has reached the end
                    lcurrent = lcurrent->next;
                current = lcurrent;
                return STATE_FOUND;
            }
            //std::cout << "expression: exp ending: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (eval_numeric_literal(lcurrent) == STATE_FOUND) // found a number literal
            {
                ////std::cout << "expression: numeric literal: " << *(lcurrent->value) << std::endl;
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
            ////std::cout << "expression: numeric literal: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            if (check_eof(lcurrent, true))
                break;
            if (application == nullptr)
                preserve_value("rax"); // preserve value in rax so a possible function call doesn't overwrite it
            if (eval_function_call(lcurrent) == STATE_FOUND)
            {
                if (application != nullptr) // if an application is provided, it will be stored at its stack location
                    location = "[rbp - " + std::to_string(application->offset) + "]";
                std::cout << "function call validated from expression with an application of ";
                if (application != nullptr)
                    std::cout << application->name() << " and a location of ";
                std::cout << location << std::endl;
                ////std::cout << "expression: function call: " << *(lcurrent->value) << std::endl;
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
                std::cout << "ending? " << *(lcurrent->value) << std::endl;
                //if (eval_exp_ending(lcurrent) != STATE_FOUND) // if this is the end, keep it on the ending for later
                //    lcurrent = lcurrent->next; // otherwise, move on
                continue;
            }
            if (application == nullptr)
                retrieve_value("rax"); // preserve value in rax so a possible function call doesn't overwrite it
            //std::cout << "expression: function call: lcurrent tracker: " << *(lcurrent->value) << std::endl;
            asc::symbol*& symbol = symbols[*(lcurrent->value)];
            if (symbol != nullptr)
            {
                std::string symbol_loc = "[rbp - " + std::to_string(symbol->offset) + "]";
                std::string res = asc::resolve_register(location, asc::get_type_size(symbol->type));
                std::cout << "lol: " << res << std::endl;
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
            //std::cout << "expression: symbol: lcurrent tracker: " << *(lcurrent->value) << std::endl;
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
        type_symbol* sym = new type_symbol(identifier, "type", symbol_types::STRUCTLIKE_TYPE, v, this->scope);
        symbols[identifier] = sym;
        while (!check_eof(lcurrent) || *(lcurrent) == "}")
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
            if (check_eof(lcurrent = lcurrent->next)) // move to semicolon or default definition
                return STATE_SYNTAX_ERROR;
            if (*(lcurrent) == ";")
            {
                syntax_node* copied_node = lcurrent;
                sym->members.push_back(copied_node);
                symbols[identifier] = new symbol(identifier, t, symbol_types::STRUCTLIKE_TYPE_MEMBER, "public", dynamic_cast<symbol*>(sym));
                continue;
            }
            if (*(lcurrent) != "=")
            {
                asc::err("expected semicolon or default definition", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            if (check_eof(lcurrent = lcurrent->next)) // move to semicolon or default definition
                return STATE_SYNTAX_ERROR;
            
        }
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

    parser::~parser()
    {
        if (scope != nullptr)
            delete scope;
    }
}