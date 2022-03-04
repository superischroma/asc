#include <stdexcept>
#include <algorithm>
#include <queue>
#include <stack>
#include <array>

#include "parser.h"

#define MAX_INT32 0x7FFFFFFF

namespace asc
{
    class storage_register;

    typedef struct
    {
        rpn_element* start;
        rpn_element* end;
    } function_argument_segment;

    symbol* invalid_symbol = nullptr;

    parser::parser(syntax_node* root)
    {
        this->current = root;
        this->scope = nullptr;
        this->branchc = 0;
        this->slc = 0;
        this->fplc = 0;
        this->dpc = 0;
        this->dpm = 0;
        // add all standard types
        for (auto& p : STANDARD_TYPES)
            this->symbols[p.first].push_back(&(p.second));
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

    evaluation_state parser::eval_function_header(syntax_node*& lcurrent, function_symbol*& result, bool use_declaration)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        // errors will be thrown later on once we CONFIRM this is supposed to be a function declaration
        evaluation_state v_state = visibilities::value_of(asc::to_uppercase(*(slcurrent->value))) != visibilities::INVALID;
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
        type_symbol* t = nullptr;
        bool t_array;
        evaluation_state t_state = eval_full_type(slcurrent, t, t_array);
        if (t_state == STATE_NEUTRAL || t_state == STATE_SYNTAX_ERROR)
            return t_state;
        //asc::debug('t' << std::endl;
        // at this point, it could still be a variable definition/declaration, so let's continue
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
        function_symbol* f_symbol = dynamic_cast<function_symbol*>(symbol_table_insert(identifier, new asc::function_symbol(identifier, t,
            t_array, symbol_variants::FUNCTION, visibilities::value_of(asc::to_uppercase(v)), scope, use_declaration)));
        result = f_symbol;
        for (int c = 1, s = 8; true; c++) // loop until we're at the end of the declaration, this is an infinite loop to make code smoother
        {
            lcurrent = lcurrent->next; // first, the argument type
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            if (*(lcurrent->value) == ")") // if there are no more arguments, leave the loop
                break;
            int at_line = lcurrent->line;
            type_symbol* at = nullptr;
            bool at_array;
            evaluation_state at_state = eval_full_type(lcurrent, at, at_array);
            if (at_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (at_state == STATE_NEUTRAL) // if there was no type specifier, throw an error
            {
                asc::err("type specifier expected for argument " + std::to_string(c), at_line);
                return STATE_SYNTAX_ERROR;
            }
            // second, the argument identifier
            if (check_eof(lcurrent))
                return STATE_SYNTAX_ERROR;
            if (*lcurrent == "," || *lcurrent == ")") // nameless argument
            {
                if (use_declaration) // we're predefining it using a use statement
                {
                    f_symbol->parameters.push_back(new asc::symbol('_' + f_symbol->m_name + "_arg" + std::to_string(c - 1), at,
                        at_array, symbol_variants::FUNCTION_VARIABLE, visibilities::PUBLIC, static_cast<symbol*>(f_symbol)));
                    if (*lcurrent == ")")
                    {
                        asc::debug("declared function with use: " + f_symbol->to_string());
                        return STATE_FOUND;
                    }
                    continue;
                }
                asc::err("nameless function arguments are not allowed", at_line);
                return STATE_SYNTAX_ERROR;
            }
            int ai_line = lcurrent->line;
            std::string& a_identifier = *(lcurrent->value); // get the identifier that MIGHT be there
            if (symbol_table_get_imm(a_identifier, f_symbol) != nullptr) // if symbol already exists in this scope
            {
                asc::err("symbol is already defined", ai_line);
                return STATE_SYNTAX_ERROR;
            }
            symbol* a_symbol = symbol_table_insert(a_identifier, new asc::symbol(a_identifier, at,
                at_array, symbol_variants::FUNCTION_VARIABLE, visibilities::PUBLIC, static_cast<symbol*>(f_symbol)));
            f_symbol->parameters.push_back(a_symbol);
            a_symbol->offset = s += 8;
            if (c <= 4 && !use_declaration)
            {
                storage_register& stor = asc::get_register(at->variant == symbol_variants::FLOATING_POINT_PRIMITIVE ?
                    FP_ARG_REGISTER_SEQUENCE[c - 1] : ARG_REGISTER_SEQUENCE[c - 1]).byte_equivalent(at->get_size());
                as.instruct(f_symbol->name(), "mov" + stor.instruction_suffix() + ' ' + at->word() + " [rbp + " +
                    std::to_string(a_symbol->offset) + "], " + stor.m_name);
            }
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
        asc::debug("defined function: " + f_symbol->to_string());
        return STATE_FOUND; // finally, return the proper state
    }

    evaluation_state parser::eval_function_header()
    {
        syntax_node* current = this->current;
        function_symbol* n = nullptr;
        return eval_function_header(current, n, false);
    }

    evaluation_state parser::eval_if_statement(syntax_node*& lcurrent)
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
        evaluation_state ev_ex = eval_expression(lcurrent = lcurrent->next);
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
        this->scope = new asc::symbol(ifbname, get_type("void"), false, symbol_variants::IF_BLOCK,
            visibilities::LOCAL, this->scope); // move scope into if statement
        current = lcurrent; // bring current up to speed
        return STATE_FOUND;
    }

    evaluation_state parser::eval_if_statement()
    {
        asc::syntax_node* current = this->current;
        return eval_if_statement(current);
    }

    evaluation_state parser::eval_while_statement(syntax_node*& lcurrent)
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
        evaluation_state ev_ex = eval_expression(lcurrent);
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
        this->scope = new asc::symbol(loopbname, get_type("void"), false, symbol_variants::WHILE_BLOCK,
            visibilities::LOCAL, this->scope); // move scope into while loop
        this->scope->helper = expression; // preserve location of expression to be evaluated later
        current = lcurrent; // bring current up to speed
        return STATE_FOUND;
    }

    evaluation_state parser::eval_while_statement()
    {
        asc::syntax_node* current = this->current;
        return eval_while_statement(current);
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
            asc::evaluation_state es_ev = eval_expression(cpy);
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
            if (*(lcurrent->value) == "native")
            {
                asc::err("unimplemented feature: native use statements", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            else
            {
                function_symbol* result = nullptr;
                auto header = eval_function_header(lcurrent, result, true);
                if (header == STATE_SYNTAX_ERROR)
                    return STATE_SYNTAX_ERROR;
                if (header == STATE_NEUTRAL)
                {
                    asc::err("function declaration is incomplete", lcurrent->line);
                    return STATE_SYNTAX_ERROR;
                }
                as.external(result->m_name);
            }
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
        type_symbol* t;
        bool t_array;
        auto t_state = eval_full_type(slcurrent, t, t_array);
        if (t_state == STATE_NEUTRAL || t_state == STATE_SYNTAX_ERROR)
            return t_state;
        if (check_eof(slcurrent, true))
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
        symbol* sym = symbol_table_insert(*(i_node->value), new asc::symbol(*(i_node->value), t, t_array,
            (scope != nullptr ? symbol_variants::LOCAL_VARIABLE : symbol_variants::GLOBAL_VARIABLE), v, scope));
        if (scope != nullptr)
        {
            sym->offset = this->reserve_data_space(t->get_size());
            stack_emulation.push_back(sym);
        }
        return eval_expression(lcurrent);
    }

    evaluation_state parser::eval_var_declaration()
    {
        syntax_node* current = this->current;
        return eval_var_declaration(current);
    }

    evaluation_state parser::eval_expression(syntax_node*& lcurrent)
    {
        std::deque<rpn_element> output;
        std::stack<expression_operator> operators;

        std::stack<function_symbol*> functions;
        std::stack<int> call_indices;
        bool call_start = false;

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
                    !functions.empty() ? functions.top() : nullptr, call_start ? !(call_start = false) : call_start });
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
                        !functions.empty() ? functions.top() : nullptr,
                        call_start ? !(call_start = false) : call_start });
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
                call_start = true;
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
                        !functions.empty() ? functions.top() : nullptr,
                        call_start ? !(call_start = false) : call_start });
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
                        !functions.empty() ? functions.top() : nullptr,
                        call_start ? !(call_start = false) : call_start });
                    operators.pop();
                }
            }
            else
            {
                output.push_back({ *(lcurrent->value), nullptr,
                    !call_indices.empty() ? call_indices.top() : -1,
                    !functions.empty() ? functions.top() : nullptr,
                    call_start ? !(call_start = false) : call_start });
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
                !functions.empty() ? functions.top() : nullptr,
                call_start ? !(call_start = false) : call_start });
            operators.pop();
        }
        
        lcurrent = lcurrent->next; // skip over the semicolon which denoted the end of the expression
        current = lcurrent; // sync up our local current with the object member

        {
            std::string db = "shunting-yard:\n";
            for (auto& it : output)
                db += it.value + " (parameter index: " + std::to_string(it.parameter_index) + ", function: " + (it.function == nullptr ? "none" : it.function->m_name) + ")\n";
                //db += it.value + ' ';
            asc::debug(db);
        }

        // reverse function call parameters
        for (auto it = output.begin(); it != output.end(); it++)
        {
            symbol* sym = symbol_table_get(it->value);
            if (sym != nullptr && sym->variant == symbol_variants::FUNCTION)
            {
                auto call = it--;
                auto* f_sym = dynamic_cast<function_symbol*>(sym);
                std::deque<rpn_element> replacement;
                int parameter_count = it->function ? it->parameter_index + 1 : 0;
                if (!f_sym->external_decl && f_sym->parameters.size() != parameter_count)
                {
                    asc::err("function " + f_sym->m_name + " expected " +
                        std::to_string(f_sym->parameters.size()) + " parameter(s), got " +
                        std::to_string(parameter_count));
                    return STATE_SYNTAX_ERROR;
                }
                // find supposed parameter count for external functions
                for (int i = 0, last = parameter_count - 1; i < parameter_count; i++)
                {
                    std::stack<rpn_element> storage;
                    for (; it->parameter_index == last || it->function != f_sym; it--)
                    {
                        storage.push(*it);
                        if (it->call_start && it->function == f_sym)
                        {
                            it--;
                            break;
                        }
                    }
                    last = it < output.begin() || it >= output.end() ? -1 : it->parameter_index;
                    for (; !storage.empty(); storage.pop())
                        replacement.push_back(storage.top());
                }
                it = output.erase(it + 1, call);
                it = output.insert(it, replacement.begin(), replacement.end());
                it += replacement.size();
            }
        }

        {
            std::string db = "shunting-yard + reverse function call:\n";
            for (auto& it : output)
                //db += it.value + " (parameter index: " + std::to_string(it.parameter_index) + ", function: " + (it.function == nullptr ? "none" : it.function->m_name) + ")\n";
                db += it.value + ' ';
            asc::debug(db);
        }

        for (auto it = output.begin(); it != output.end(); it++)
        {
            auto* element = &*it;
            std::string* token = &(element->value);
            symbol* sym = symbol_table_get(*token);
            if (OPERATORS.count(*token)) // operator
            {
                auto& oper = OPERATORS[*token];
                // addition operator
                if (oper.value == "+")
                {
                    if (oper.operands == 2)
                    {
                        symbol* fpl = floating_point_stack();
                        auto& first = retrieve_value(get_register(fpl ? "xmm5" : "rbx"));
                        auto& second = retrieve_value(get_register(fpl ? "xmm4" : "rax"));
                        as.instruct(scope->name(), "add" + (fpl != nullptr ? fpl->instruction_suffix() : "") + ' ' + second.m_name + ", " + first.m_name);
                        preserve_value(second, fpl ? fpl->get_size() : -1);
                        (it = output.erase(it))--;
                    }
                }
                // subtraction operator
                if (oper.value == "-")
                {
                    if (oper.operands == 2)
                    {
                        symbol* fpl = floating_point_stack();
                        auto& first = retrieve_value(get_register(fpl ? "xmm5" : "rbx"));
                        auto& second = retrieve_value(get_register(fpl ? "xmm4" : "rax"));
                        as.instruct(scope->name(), "sub" + (fpl != nullptr ? fpl->instruction_suffix() : "") + ' ' + second.m_name + ", " + first.m_name);
                        preserve_value(second, fpl ? fpl->get_size() : -1);
                        (it = output.erase(it))--;
                    }
                }
                // multiplication and pointer operator
                if (oper.value == "*")
                {
                    if (oper.operands == 2)
                    {
                        symbol* fpl = floating_point_stack();
                        auto& first = retrieve_value(get_register(fpl ? "xmm5" : "rbx"));
                        auto& second = retrieve_value(get_register(fpl ? "xmm4" : "rax"));
                        as.instruct(scope->name(), std::string(fpl == nullptr ? "i" : "") + "mul" +
                            (fpl != nullptr ? fpl->instruction_suffix() : "") + ' ' + second.m_name + ", " + first.m_name);
                        preserve_value(second, fpl ? fpl->get_size() : second.get_size());
                        (it = output.erase(it))--;
                    }
                    if (oper.operands == 1)
                    {
                        asc::err("pointers have not been implemented yet");
                        return STATE_SYNTAX_ERROR;
                    }
                }
                // division operator
                if (oper.value == "/")
                {
                    if (oper.operands == 2)
                    {
                        symbol* fpl = floating_point_stack();
                        auto& first = retrieve_value(get_register(fpl ? "xmm5" : "rbx"));
                        auto& second = retrieve_value(get_register(fpl ? "xmm4" : "rax"));
                        if (fpl)
                        {
                            as.instruct(scope->name(), "div" + fpl->instruction_suffix() + ' ' + second.m_name + ", " + first.m_name);
                            preserve_value(second, fpl ? fpl->get_size() : -1);
                        }
                        else
                        {
                            auto& d = get_register("rdx").byte_equivalent(second.get_size());
                            as.instruct(scope->name(), "xor " + d.m_name + ", " + d.m_name);
                            as.instruct(scope->name(), "idiv " + first.m_name);
                            preserve_value(first.get_size() != 1 ? second : get_register("al"),
                                fpl ? fpl->get_size() : -1);
                        }
                        (it = output.erase(it))--;
                    }
                }
                // division operator
                if (oper.value == "%")
                {
                    if (oper.operands == 2)
                    {
                        auto& first = retrieve_value(get_register("rbx"));
                        auto& second = retrieve_value(get_register("rax"));
                        auto& d = get_register("rdx").byte_equivalent(second.get_size());
                        as.instruct(scope->name(), "xor " + d.m_name + ", " + d.m_name);
                        as.instruct(scope->name(), "idiv " + first.m_name);
                        preserve_value(first.get_size() != 1 ? get_register("rdx").byte_equivalent(second.get_size())
                            : get_register("ah"));
                        (it = output.erase(it))--;
                    }
                }
                // assignment operator
                if (oper.value == "=")
                {
                    if (oper.operands == 2)
                    {
                        int fz = 0;
                        if (!stack_emulation.empty())
                        {
                            auto* t = stack_emulation.back();
                            std::cout << "fp test: " << (t ? t->to_string() : "null") << std::endl;
                            symbol* s = dynamic_cast<symbol*>(t);
                            fp_register* fp_reg = dynamic_cast<fp_register*>(t);
                            if (s != nullptr && s->type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE)
                                fz = s->get_size();
                            else if (fp_reg != nullptr)
                                fz = fp_reg->effective_sizes.top();
                        }
                        auto& src = retrieve_value(get_register(fz ? "xmm4" : "rax"));
                        forget_top();
                        as.instruct(scope->name(), "mov" + (fz ? std::string("s") + (fz == 8 ? 'd' : 's') : "") + ' ' + asc::relative_dereference("rbp", dynamic_cast<symbol*>(stack_emulation.back())->offset) + ", " + src.m_name);
                        preserve_value(src, fz ? fz : -1);
                        (it = output.erase(it))--;
                    }
                }
                // casting operator
                if (oper.value == "=>")
                {
                    if (oper.operands == 2)
                    {
                        type_symbol* dest_type = dynamic_cast<type_symbol*>(stack_emulation.back());
                        if (dest_type == nullptr)
                        {
                            asc::err("expected a type on right hand side of casting operator");
                            return STATE_SYNTAX_ERROR;
                        }
                        if (!dest_type->is_primitive())
                        {
                            asc::err("casting to object types is not allowed yet");
                            return STATE_SYNTAX_ERROR;
                        }
                        stack_emulation.pop_back();
                        bool is_double = dest_type->m_name == "double";
                        bool fp_dest = dest_type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE;
                        stackable_element* convertee = stack_emulation.back();
                        integral_literal* il = dynamic_cast<integral_literal*>(convertee);
                        storage_register* reg = dynamic_cast<storage_register*>(convertee);
                        symbol* sym = dynamic_cast<symbol*>(convertee);
                        storage_register* temp_dest = nullptr;

                        asc::debug("casting candidate: " + convertee->to_string() + ", to: " + dest_type->to_string());

                        if (il != nullptr || (sym != nullptr && sym->type->variant == symbol_variants::INTEGRAL_PRIMITIVE) || (reg != nullptr && !reg->is_fp_register()))
                        {
                            // retrieve value with sign extension if necessary
                            temp_dest = &(retrieve_value(get_register("rax").byte_equivalent(fp_dest ? 8 : dest_type->get_size()), // integral -> integral
                                false, false, dest_type->get_size() > (il ? il->get_size() : (reg ? reg->get_size() : sym->get_size())), true));
                            std::cout << "temp_dest: " << temp_dest->to_string() << std::endl;
                            if (fp_dest) // integral -> float/double
                            {
                                // being converted to floating point
                                temp_dest = &(get_register("xmm4"));
                                as.instruct(scope->name(), std::string("cvtsi2s") + (is_double ? 'd' : 's')
                                    + " xmm4, rax");
                            }
                        }
                        else if ((sym != nullptr && sym->type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE) || (reg != nullptr && reg->is_fp_register()))
                        {
                            if (fp_dest) // float/double -> double/float
                            {
                                temp_dest = &(get_register("xmm4"));
                                int size;
                                storage_register* loc = nullptr;
                                if (sym != nullptr && sym->name_identified)
                                {
                                    as.instruct(scope->name(), "mov rax, " + sym->location());
                                    size = sym->get_size();
                                }
                                else
                                    loc = &(retrieve_value(get_register("xmm5"), false, false, false, false, &size));
                                std::cout << "covid vaccine: " << size << std::endl;
                                if ((size == 8 && is_double) || (size != 8 && !is_double))
                                {
                                    as.instruct(scope->name(), std::string("movs") + (size == 8 ? 'd' : 's') +
                                        ' ' + temp_dest->m_name + ", " + (sym != nullptr ? 
                                        (sym->name_identified ? sym->word() + " [rax]" : sym->location()) : loc->m_name));
                                }
                                else
                                {
                                    as.instruct(scope->name(), std::string("cvts") + (size == 8 ? 'd' : 's') +
                                        "2s" + (is_double ? 'd' : 's') + ' ' + temp_dest->m_name + ", " +
                                        (sym != nullptr ? (sym->name_identified ? sym->word() + " [rax]" : sym->location()) : loc->m_name));
                                }
                            }
                            else // float/double -> integral
                            {
                                temp_dest = &(get_register("rax"));
                                int size;
                                retrieve_value(get_register("xmm4"), false, false, false, false, &size);
                                as.instruct(scope->name(), std::string("cvtts") + (size == 8 ? 'd' : 's') +
                                    "2si rax, xmm4");
                                temp_dest = &(temp_dest->byte_equivalent(dest_type->get_size()));
                            }
                        }
                        else
                        {
                            asc::err("left hand side of cast is not a candidate for casting");
                            return STATE_SYNTAX_ERROR;
                        }
                        std::cout << temp_dest << std::endl;
                        preserve_value(*temp_dest, dest_type->get_size());
                        (it = output.erase(it))--;
                    }
                }
            }
            else if (sym != nullptr && sym->variant == symbol_variants::FUNCTION) // function call
            {
                auto* f_sym = dynamic_cast<function_symbol*>(sym);
                asc::debug("calling: " + f_sym->to_string());
                for (int i = 0; i < (f_sym->parameters.size() > 4 ? 4 : f_sym->parameters.size()); i++)
                    retrieve_value(get_register(f_sym->parameters[i]->type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE ?
                        FP_ARG_REGISTER_SEQUENCE[i] : ARG_REGISTER_SEQUENCE[i]).byte_equivalent(f_sym->parameters[i]->get_size()), false, true);
                int s_arg_count = f_sym->parameters.size() - 4;
                for (int i = 0, h = 0; i < s_arg_count; i++)
                {
                    int top_size = stack_emulation.back()->get_size();
                    auto& transfer = get_register("rax").byte_equivalent(top_size);
                    as.instruct(scope->name(), "mov " + transfer.m_name + ", " + top_location());
                    forget_top();
                    as.instruct(scope->name(), "mov [rsp + " +
                        std::to_string(i + h + 32) + "], " + transfer.m_name);
                    h += top_size;
                }
                as.instruct(scope->name(), "call " + f_sym->m_name);
                if (f_sym->get_size() != 0)
                    preserve_value(get_register(f_sym->type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE ? "xmm0" : "rax").byte_equivalent(f_sym->get_size()), f_sym->get_size()); // preserve the return value
                (it = output.erase(it))--;
            }
            else if (sym != nullptr) // variable
            {
                if (dynamic_cast<type_symbol*>(sym) != nullptr) // if it's a type symbol
                    stack_emulation.push_back(sym);
                else
                    preserve_symbol(sym);
                (it = output.erase(it))--;
            }
            else if (is_string_literal(*token)) // string literal
            {
                symbol* str = symbol_table_insert("_SL" + std::to_string(slc),
                    new symbol("_SL" + std::to_string(slc), get_type("char"), true, symbol_variants::GLOBAL_VARIABLE, visibilities::PRIVATE, nullptr));
                str->name_identified = true;
                as << asc::data << str->m_name + " db " + *token + ", 0x00";
                slc++;
                preserve_symbol(str);
                (it = output.erase(it))--;
            }
            else if (is_number_literal(*token, true)) // integral constants
            {
                as.instruct(scope->name(), "mov dword " + asc::relative_dereference("rbp", reserve_data_space(4)) + ", " + *token); // temporary
                integral_literal* il = new integral_literal(4);
                il->dynamic = true; 
                stack_emulation.push_back(il);
                (it = output.erase(it))--;
            }
            else if (is_number_literal(*token)) // floating point constants
            {
                bool is_double = is_double_literal(*token);
                symbol* fpl = symbol_table_insert("_FPL" + std::to_string(fplc),
                    new symbol("_FPL" + std::to_string(fplc), get_type(is_double ? "double" : "float"),
                        false, symbol_variants::GLOBAL_VARIABLE, visibilities::PRIVATE, nullptr));
                fpl->name_identified = true;
                as << asc::data << fpl->m_name + " d" + (is_double ? "q " : "d ") + strip_number_literal(*token);
                as.instruct(scope->name(), "mov rax, " + fpl->m_name);
                as.instruct(scope->name(), std::string("movs") + (is_double ? 'd' : 's') + " xmm4, " + fpl->type->word() + " [rax]");
                fplc++;
                preserve_symbol(fpl);
                (it = output.erase(it))--;
            }
            else
            {
                asc::err("invalid token encountered while parsing expression");
                return STATE_SYNTAX_ERROR;
            }
        }

        // asc -experimental -debug -symbolize tests/expressions.as

        return STATE_FOUND;
    }

    evaluation_state parser::eval_expression()
    {
        syntax_node* current = this->current;
        return eval_expression(current);
    }

    evaluation_state parser::eval_return_statement(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        if (*lcurrent != "return")
            return STATE_NEUTRAL;
        if (scope == nullptr)
        {
            asc::err("return statement outside of function", lcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        auto exp = eval_expression(lcurrent = lcurrent->next);
        if (exp != STATE_FOUND)
            return exp;
        retrieve_value(get_register(get_current_function()->type->variant != symbol_variants::FLOATING_POINT_PRIMITIVE ? "rax" : "xmm0"));
        return STATE_FOUND;
    }

    evaluation_state parser::eval_return_statement()
    {
        syntax_node* current = this->current;
        return eval_return_statement(current);
    }

    evaluation_state parser::eval_type_construct(syntax_node*& lcurrent)
    {
        if (check_eof(lcurrent, true))
            return STATE_NEUTRAL;
        syntax_node* slcurrent = lcurrent;
        evaluation_state v_state = visibilities::value_of(asc::to_uppercase(*(slcurrent->value))) != visibilities::INVALID;
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
        type_symbol* sym = new type_symbol(identifier, get_type("void"), false, symbol_variants::STRUCTLIKE_TYPE,
            visibilities::value_of(asc::to_uppercase(v)), 0, this->scope);
        symbol_table_insert(identifier, sym);
        int overall_size = 0; // keep track of type's size
        while (!check_eof(lcurrent) && *(lcurrent) != "}")
        {
            int t_line = lcurrent->line;
            type_symbol* t;
            bool t_array;
            evaluation_state t_state = eval_full_type(lcurrent, t, t_array);
            if (t_state == STATE_SYNTAX_ERROR)
                return STATE_SYNTAX_ERROR;
            if (t_state == STATE_NEUTRAL)
            {
                asc::err("type expected", t_line);
                return STATE_SYNTAX_ERROR;
            }
            if (check_eof(lcurrent = lcurrent->next)) // move to identifier
                return STATE_SYNTAX_ERROR;
            std::string identifier = *(lcurrent->value);
            if (lcurrent->type != syntax_types::IDENTIFIER)
            {
                asc::err("identifier expected", lcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            syntax_node* identifier_node = lcurrent;
            symbol* member_symbol = new symbol(identifier, t, t_array, symbol_variants::STRUCTLIKE_TYPE_MEMBER,
                visibilities::PUBLIC, static_cast<symbol*>(sym));
            sym->members.push_back(member_symbol);
            symbol_table_insert(identifier, member_symbol);
            overall_size += t->get_size();
            while (!check_eof(lcurrent = lcurrent->next) && *(lcurrent) != ";");
            if (lcurrent == nullptr)
                return STATE_SYNTAX_ERROR;
            if (check_eof(lcurrent = lcurrent->next)) // skip semicolon
                return STATE_SYNTAX_ERROR;
        }
        sym->size = overall_size; // update type's size with the size of its members
        current = lcurrent = lcurrent->next;
        return STATE_FOUND;
    }

    evaluation_state parser::eval_type_construct()
    {
        syntax_node* current = this->current;
        return eval_type_construct(current);
    }

    evaluation_state parser::eval_full_type(syntax_node*& lcurrent, type_symbol*& found, bool& array)
    {
        syntax_node* slcurrent = lcurrent;
        if (check_eof(slcurrent, true))
            return STATE_NEUTRAL;
        unsigned char s = 0;
        if (*slcurrent == "signed") s = 1;
        if (*slcurrent == "unsigned") s = 2;
        if (s != 0)
        {
            if (check_eof(slcurrent = slcurrent->next, true))
                return STATE_NEUTRAL;
        }
        std::string identifier = (s == 2 ? "u" : "") + *(slcurrent->value);
        asc::symbol* type = symbol_table_get(identifier);
        if (type == nullptr)
            return STATE_NEUTRAL;
        if (type->variant != symbol_variants::OBJECT &&
                type->variant != symbol_variants::STRUCTLIKE_TYPE &&
                type->variant != symbol_variants::PRIMITIVE &&
                type->variant != symbol_variants::INTEGRAL_PRIMITIVE &&
                type->variant != symbol_variants::UNSIGNED_INTEGRAL_PRIMITIVE &&
                type->variant != symbol_variants::FLOATING_POINT_PRIMITIVE)
            return STATE_NEUTRAL;
        if (s == 1 && type->variant != symbol_variants::INTEGRAL_PRIMITIVE)
        {
            asc::err("cannot apply modifier 'signed' to type '" + type->m_name + '\'', slcurrent->line);
            return STATE_SYNTAX_ERROR;
        }
        found = dynamic_cast<type_symbol*>(type);
        slcurrent = slcurrent->next;
        array = false;
        if (!check_eof(slcurrent, true) && *slcurrent == "[")
        {
            if (check_eof(slcurrent->next, true) || *(slcurrent->next) != "]")
            {
                asc::err("expected right closing bracket for array type", slcurrent->line);
                return STATE_SYNTAX_ERROR;
            }
            array = true;
            slcurrent = slcurrent->next->next;
        }
        lcurrent = slcurrent;
        return STATE_FOUND;
    }

    int parser::preserve_value(storage_register& location, int size, symbol* scope)
    {
        if (size == -1)
            size = location.get_size();
        fp_register* fpr = dynamic_cast<fp_register*>(&location);
        if (fpr != nullptr)
        {
            asc::debug("effective size added for register " + location.m_name + ": " + std::to_string(size));
            fpr->effective_sizes.push(size);
        }
        int position = (dpc += size);
        if (dpc > dpm) dpm = dpc; // update max if needed
        stack_emulation.push_back(&location);
        as.instruct(scope != nullptr ? scope->name() : this->scope->name(), "mov" +
            (fpr ? std::string("s") + (size == 4 ? 's' : 'd') : "") + ' ' + asc::relative_dereference("rbp", -position, asc::word(size)) + ", " + location.m_name);
        return -position;
    }

    int parser::preserve_symbol(symbol* sym, symbol* scope)
    {
        int position = (dpc += sym->get_size());
        if (dpc > dpm) dpm = dpc; // update max if needed
        stack_emulation.push_back(sym);
        storage_register& transfer_register = get_register("rax").byte_equivalent(sym->get_size());
        as.instruct(scope != nullptr ? scope->name() : this->scope->name(), "mov " + transfer_register.m_name + ", " + sym->location());
        as.instruct(scope != nullptr ? scope->name() : this->scope->name(), "mov " + asc::relative_dereference("rbp", -position, sym->word()) + ", " + transfer_register.m_name);
        return -position;
    }

    int parser::reserve_data_space(int size)
    {
        int position = (dpc += size);
        if (dpc > dpm) dpm = dpc; // update max if needed
        return -position;
    }
    /**
     * @brief Takes a value off of the stack in emulation and output and stores it in the storage argument
     * 
     * @param storage The location to store the value in output
     * @param lea Whether the memory address should be stored rather than the value
     * @param cc Whether this function call might need to follow x64 calling convention rules
     * @param sx Whether the value should be sign extended once it is retrieved
     * @param use_passed_storage Whether it should not check for a proper storage register extension and use the one provided by the callee
     * @param size Reference to a variable which stores the size the register used
     * @return storage_register& The actual register it was stored in if the function chose an larger or smaller extension register
     */
    storage_register& parser::retrieve_value(storage_register& storage, bool lea, bool cc, bool sx, bool use_passed_storage, int* size)
    {
        stackable_element* element = stack_emulation.back();
        symbol* sym = dynamic_cast<symbol*>(element);
        fp_register* fp_storage = dynamic_cast<fp_register*>(element);
        int lsize = !sx && !use_passed_storage && fp_storage ? fp_storage->effective_sizes.top() : element->get_size();
        storage_register& dest = sx || use_passed_storage ? storage : storage.byte_equivalent(lsize);
        std::cout << dest.to_string() << ", " << sx << ", " << lsize << std::endl;
        if (!sx && !use_passed_storage && fp_storage)
            fp_storage->effective_sizes.pop();
        storage_register& dest64 = dest.byte_equivalent(8);
        if (dest.get_size() != 8 && !dest.is_fp_register())
            as.instruct(scope->name(), "xor " + dest64.m_name + ", " + dest64.m_name);
        std::string src = (sym != nullptr && sym->name_identified) ? sym->m_name :
                asc::relative_dereference("rbp", sym != nullptr ? sym->offset : -dpc, sx ? element->word() : (fp_storage ? word(lsize) : dest.word()));
        bool dereference_needed = sym != nullptr && sym->name_identified && dest.is_fp_register();
        if (dereference_needed)
            as.instruct(scope->name(), "mov rax, " + sym->m_name);
        as.instruct(scope->name(), std::string("mov" + (sx ? "sx" :
            (dest.is_fp_register() && sym != nullptr ? sym->instruction_suffix() : (fp_storage ? std::string("s") + (lsize == 4 ? 's' : 'd') : "")))) +
            ' ' + dest.m_name + ", " + (dereference_needed ? (sx ? element->word() : dest.word()) + " [rax]" : src));
        auto sequence_index = std::find(FP_ARG_REGISTER_SEQUENCE.begin(), FP_ARG_REGISTER_SEQUENCE.end(), dest.m_name);
        if (cc && sequence_index != FP_ARG_REGISTER_SEQUENCE.end())
        {
            as.instruct(scope->name(), std::string("mov") + ' ' +
                get_register(ARG_REGISTER_SEQUENCE[std::distance(FP_ARG_REGISTER_SEQUENCE.begin(), sequence_index)]).byte_equivalent(lsize).m_name +
                ", " + (dereference_needed ? (sx ? element->word() : (fp_storage ? word(lsize) : dest.word())) + " [rax]" : src));
        }
        dpc -= lsize;
        if (element->dynamic)
            delete element;
        stack_emulation.pop_back();
        if (size != nullptr)
            *size = lsize;
        return dest;
    }

    std::string parser::top_location()
    {
        return asc::relative_dereference("rbp", -dpc);
    }

    void parser::forget_top()
    {
        stackable_element* element = stack_emulation.back();
        dpc -= element->get_size();
        if (element->dynamic)
            delete element;
        stack_emulation.pop_back();
    }

    bool parser::symbol_table_has(std::string name, symbol* scope)
    {
        try
        {
            symbol_table_get(name, scope);
            return true;
        }
        catch (std::out_of_range& e)
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
     * @return symbol*
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
        catch (std::out_of_range& e)
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

    symbol* parser::get_current_function()
    {
        symbol* current = this->scope;
        for (; current != nullptr && current->variant != symbol_variants::FUNCTION; current = current->scope);
        return current;
    }

    symbol* parser::floating_point_stack(int argc)
    {
        auto ending = stack_emulation.end() - argc;
        for (auto it = stack_emulation.end() - 1; it >= ending; it--)
        {
            symbol* sym = dynamic_cast<symbol*>(*it);
            if (sym != nullptr && sym->type->variant == symbol_variants::FLOATING_POINT_PRIMITIVE)
                return sym;
        }
        return nullptr;
    }

    type_symbol* parser::get_type(std::string str)
    {
        symbol* sym = symbol_table_get(str);
        if (sym == nullptr)
            return nullptr;
        return (sym->variant == symbol_variants::STRUCTLIKE_TYPE ||
            sym->variant == symbol_variants::OBJECT ||
            sym->variant == symbol_variants::PRIMITIVE ||
            sym->variant == symbol_variants::FLOATING_POINT_PRIMITIVE) ? dynamic_cast<type_symbol*>(sym) : nullptr;
    }

    parser::~parser()
    {
        delete scope;
    }
}