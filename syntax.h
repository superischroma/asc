#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

#include <string>
#include <type_traits>

#include "assembler.h"
#include "symbol.h"
#include "logger.h"

namespace asc
{
    namespace syntax_types
    {
        const unsigned short PROGRAM_BEGIN = 0;
        const unsigned short KEYWORD = 1;
        const unsigned short IDENTIFIER = 2;
        const unsigned short PUNCTUATOR = 3;
        const unsigned short CONSTANT = 4;

        std::string name(unsigned short type)
        {
            switch (type)
            {
                case 0: return "PROGRAM_BEGIN";
                case 1: return "KEYWORD";
                case 2: return "IDENTIFIER";
                case 3: return "PUNCTUATOR";
                case 4: return "CONSTANT";
            }
            return std::to_string(type);
        }
    }

    const char* KEYWORDS[] = {
        "void",
        "byte",
        "bool",
        "char",
        "short",
        "int",
        "long",
        "float",
        "double",
        "signed",
        "unsigned",
        "return",
        "public",
        "private",
        "protected"
    };

    const char* PUNCTUATORS[] = {
        "{",
        "}",
        "(",
        ")",
        ";",
        "=",
        ",",
        "+",
        "-",
        "*",
        "/"
    };

    const char* OPERATORS[] = {
        "=",
        "+",
        "-",
        "*",
        "/",
        "%"
    };

    int strlen(const char* str)
    {
        int length = 0;
        for (; str[length] != '\0'; length++);
        return length;
    }

    unsigned char is_keyword(std::string& test)
    {
        for (int i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++)
        {
            if (test.length() < strlen(KEYWORDS[i]))
                continue;
            if (KEYWORDS[i] == test.substr(test.length() - strlen(KEYWORDS[i])))
                return strlen(KEYWORDS[i]);
        }
        return 0;
    }

    unsigned char is_punctuator(std::string& test)
    {
        for (int i = 0; i < sizeof(PUNCTUATORS) / sizeof(PUNCTUATORS[0]); i++)
        {
            if (test.length() < strlen(PUNCTUATORS[i]))
                continue;
            if (PUNCTUATORS[i] == test.substr(test.length() - strlen(PUNCTUATORS[i])))
                return strlen(PUNCTUATORS[i]);
        }
        return 0;
    }

    int get_operator(std::string& test)
    {
        for (int i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); i++)
        {
            if (OPERATORS[i] == test)
                return i;
        }
        return -1;
    }

    char get_visibility_id(std::string& test)
    {
        if (test == "public")
            return 0;
        if (test == "private")
            return 1;
        if (test == "protected")
            return 2;
        return -1;
    }

    bool is_numerical(std::string& test)
    {
        int i = 0;
        for (; test[i] == '0' ||
            test[i] == '1' ||
            test[i] == '2' ||
            test[i] == '3' ||
            test[i] == '4' ||
            test[i] == '5' ||
            test[i] == '6' ||
            test[i] == '7' ||
            test[i] == '8' ||
            test[i] == '9' ||
            test[i] == '.'; i++);
        return i == test.length();
    }

    bool is_primitive(std::string& test)
    {
        return test == "void" ||
            test == "byte" ||
            test == "bool" ||
            test == "char" ||
            test == "short" ||
            test == "int" ||
            test == "long" ||
            test == "float" ||
            test == "double";
    }

    int get_type_size(std::string& prim)
    {
        if (prim == "void")
            return 0;
        if (prim == "byte" || prim == "bool")
            return 1;
        if (prim == "char" || prim == "short")
            return 2;
        if (prim == "int" || prim == "float")
            return 4;
        return 8;
    }

    std::string get_word(int size)
    {
        if (size == 1)
            return "byte";
        if (size == 2)
            return "word";
        if (size == 4)
            return "dword";
        if (size == 8)
            return "qword";
        return "-1";
    }

    class syntax_node
    {
    public:
        syntax_node* next;
        unsigned short type;
        std::string* value;
        int line;
        syntax_node(syntax_node* next, unsigned short type, std::string value, int line)
        {
            this->next = next;
            this->type = type;
            this->value = new std::string(value);
            this->line = line;
        }
        std::string stringify()
        {
            return "syntax_node{type=" + syntax_types::name(type) + ", value=" + (value != nullptr ? *value : "") + ", line=" + std::to_string(line) + "}";
        }
        ~syntax_node()
        {
            delete next;
            delete value;
        }
    };

    /*
    bool parse_expression(syntax_node*& current, assembler& as, std::map<std::string, symbol*>& symbols, symbol*& scope, symbol* sym, bool declaration)
    {
        std::string datast = (sym != nullptr ? *(sym->name) + " db " : "");
        int datastl = datast.length();
        for (; current != nullptr; current = current->next)
        {
            std::string& value = *(current->value);
            if (value == ";" || value == "{" || value == "}" || value == "," || value.length() == 0) // if we reached the end of the expression
                break; // break out of the loop
            if (value == "=")
            {
                parse_expression(current = current->next, as, symbols, scope, sym, declaration);
                break;
            }
            else if (is_numerical(value)) // basic numerical checking for now
            {
                if (scope == nullptr) // if we're in global scope
                {
                    if (sym == nullptr) // if we aren't parsing an expression for a symbol
                    {
                        asc::err("stranded constant", current->line);
                        return false;
                    }
                    if (datastl != datast.length()) datast += ", ";
                    datast += value;
                }
                else
                {
                    if (sym != nullptr) // if this is not a standalone constant
                    {
                        int size = get_type_size(sym->type);
                        sym->stack_m = scope->stack_m -= size; // store stack location for identifier and move down stack tracker for scope
                        as.alloc_delta(*(scope->name), size);
                        std::cout << "declaration: " << *(scope->name) << ", " << *(sym->name) << ", " << sym->type << ", " << size << std::endl;
                        as.instruct(*(scope->name), "mov " + get_word(size) + " [rbp" + std::to_string(sym->stack_m) + "], " + value);
                    }
                    else
                        as.instruct(*(scope->name), "mov rax, " + value);
                }
            }
            // BINARY OPERATORS
            else if (value == "+")
            {
                if (current->next == nullptr) // incomplete math operation
                {
                    asc::err("incomplete statement", current->line);
                    return false;
                }
                syntax_node* op2 = current->next;
            }
            else if (value == "(" && sym != nullptr) // marks function or function call
            {
                if (current->next == nullptr) // incomplete function/function call
                {
                    asc::err("incomplete statement", current->line);
                    return false;
                }
                current = current->next;
                if (*(current->value) == ")" && !declaration) // if 0 args are present
                {
                    as.instruct(*(scope->name), "call " + *(sym->name)); // just call it
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
                            s = new symbol(*(param->value), type, sym);
                            if (i < 4)
                                as.instruct(*(sym->name), "mov " + get_word(size) + " [rbp + " + std::to_string((i * 8) + 16) + "], " + resolve_register(ARG_REGISTER_SEQUENCE[i], size));
                            i++;
                        }
                        else
                        {
                            asc::warn("uncompleted action made: calling non-0-arg function");
                            parse_expression(current = current->next, as, symbols, scope, sym, declaration);
                        }
                    }
                    if (declaration)
                        scope = sym; // move current scope into function
                }
            }
        }
        if (current == nullptr) // if we reached the end of the tokens without an end to the expression
        {
            asc::err("incomplete expression", current->line);
            return false;
        }
        if (datastl != datast.length())
            as << asc::data << datast;
        return true;
        /*
        std::string& usage = *(current->next->value); // how will this symbol be used?
        if (usage == "(") // is it used as a function declaration or function call?
        {
            if (type.length() == 0) // if there is no type declaration, then it's probably a call
            {
                asc::syntax_node* pass;
                for (pass = current->next->next; pass != nullptr; pass = pass->next) // iterate through arguments
                {
                    if (*(pass->value) == ")") // are we at end of the function call?
                        break;
                    if (*(pass->value) == ",")
                        break;
                    asc::parse_expression(pass, as, symbols);
                }
                if (pass == nullptr) // did we reach the end of the tokens without reaching a right parenthesis?
                { // if so, it's probably an incomplete function call
                    asc::err("incomplete function call", current->line);
                    return -1;
                }
                current = pass;
            }
        }
        else if (usage == "=") // is it used as 
        {
            
        }
        
    }
    */
}

#endif