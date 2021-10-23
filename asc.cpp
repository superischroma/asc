#include <iostream>
#include <fstream>
#include <map>

#include "syntax.h"
#include "tokenizer.h"
#include "logger.h"
#include "symbol.h"
#include "assembler.h"

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        asc::err("no input files");
        return -1;
    }
    std::ifstream is = std::ifstream(argv[1]);
    asc::tokenizer tk = asc::tokenizer(is);
    for (std::string token; tk.extractable();)
        tk >> token;
    for (asc::syntax_node* current = tk.syntax_start(); current != nullptr; current = current->next)
        std::cout << current->stringify() << std::endl;
    std::map<std::string, asc::symbol*> symbols;
    // variables for keeping track of certain properties
    char visibility = -1;
    std::string type;
    asc::symbol* current_scope = nullptr;
    // for helping create the assembly code for exporting
    asc::assembler as;
    for (asc::syntax_node* current = tk.syntax_start(); current != nullptr; current = current->next)
    {
        std::string& value = *(current->value);
        if (value.length() == 0)
            continue;
        if (value == "{" || value == ";" || value.length() == 0)
            continue;
        if (value == "}")
        {
            if (current_scope == nullptr)
            {
                asc::err("attempt to scope out of global scope", current->line);
                return -1;
            }
            current_scope = current_scope->scope;
            continue;
        }
        if (value == "return")
        {
            if (current->next == nullptr)
            {
                asc::err("semicolon or expression expected", current->line);
                return -1;
            }
            if (!asc::parse_expression(current->next, as, symbols, current_scope, nullptr, false))
                return -1;
            as.instruct(*(current_scope->name), "pop rax");
            as.instruct(*(current_scope->name), "ret");
            continue;
        }
        char v_id = asc::get_visibility_id(value);
        if (v_id != -1 && visibility != -1)
        {
            asc::err("visibility for this symbol has already been specified", current->line);
            return -1;
        }
        if (v_id != -1)
        {
            visibility = v_id;
            continue;
        }
        bool prim = asc::is_primitive(value);
        if (prim && type.length() != 0)
        {
            asc::err("type for this symbol has already been specified", current->line);
            return -1;
        }
        if (prim)
        {
            type = value;
            continue;
        }
        asc::symbol*& symbol = symbols[value];
        bool declaration = type.length() != 0;
        if (symbol != nullptr && declaration) // it's either being used or being redefined
        // check if a type has been specified for it (it's probably being redefined)
        {
            asc::err("symbol has already been defined", current->line);
            return -1;
        }
        if (symbol == nullptr && !declaration)
        // check if no type has been specified and it's not defined
        {
            asc::err("symbol has not been defined", current->line);
            return -1;
        }
        if (visibility == -1) // if no visibility was specified for this member
            visibility = 0; // make it public by default
        symbol = new asc::symbol(value, type, current_scope);
        visibility = -1;
        type.erase();
        if (current->next == nullptr)
        {
            asc::err("semicolon or declaration expected", current->line);
            return -1;
        }
        if (*(current->next->value) != ";")
        {
            if (!asc::parse_expression(current = current->next, as, symbols, current_scope, symbol, declaration))
                return -1;
        }
        std::cout << "parsed yay" << std::endl;
    }
    asc::symbol*& entry = symbols["main"];
    if (entry == nullptr)
    {
        asc::err("no entry point found in program");
        return -1;
    }
    std::ofstream os = std::ofstream(std::string(argv[1]) + ".asm", std::ios::trunc);
    std::string constructed = as.construct();
    os.write(constructed.c_str(), constructed.length());
    os.close();
}