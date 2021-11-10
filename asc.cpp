#include <iostream>
#include <fstream>
#include <map>

#include "syntax.h"
#include "tokenizer.h"
#include "logger.h"
#include "parser.h"
#include "util.h"
#include "cli.h"
#include "asc.h"

int main(int argc, char* argv[])
{
    asc::arg_result args = asc::eval_args(argc, argv);
    if (args.files.size() <= 0)
    {
        asc::err("no input files");
        return -1;
    }
    if (asc::has_option_set(args, asc::cli_options::TOKENIZE))
    {
        for (auto const& file : args.files)
        {
            if (asc::visually_tokenize(file) == -1)
                return -1;
        }
        return 0;
    }
    for (auto const& file : args.files)
    {
        if (asc::compile(std::string(argv[1])) == -1)
            return -1;
    }
    return 0;
}
namespace asc
{
    int compile(std::string filepath)
    {
        if (!asc::ends_with(filepath, ".as"))
        {
            asc::err(filepath + " is not A# source code");
            return -1;
        }
        std::ifstream is = std::ifstream(filepath);
        asc::tokenizer tk = asc::tokenizer(is);
        for (std::string token; tk.extractable();)
            tk >> token;
        for (asc::syntax_node* current = tk.syntax_start(); current != nullptr; current = current->next)
            std::cout << current->stringify() << std::endl;
        asc::parser ps = asc::parser(tk.syntax_start());
        while (ps.parseable())
        {
            std::cout << "token: " << *(ps.current->value) << std::endl;
            asc::evaluation_state es_ue = ps.eval_use();
            std::cout << "use: " << (int) es_ue << std::endl;
            if (es_ue == asc::STATE_FOUND)
                continue;
            if (es_ue == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_be = ps.eval_block_ending();
            std::cout << "block ending: " << (int) es_be << std::endl;
            if (es_be == asc::STATE_FOUND)
                continue;
            if (es_be == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fd = ps.eval_function_decl();
            std::cout << "function decl: " << (int) es_fd << std::endl;
            if (es_fd == asc::STATE_FOUND)
                continue;
            if (es_fd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_vdd = ps.eval_variable_decl_def();
            std::cout << "variable: " << (int) es_vdd << std::endl;
            if (es_vdd == asc::STATE_FOUND)
                continue;
            if (es_vdd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_r = ps.eval_ret();
            std::cout << "return: " << (int) es_r << std::endl;
            if (es_r == asc::STATE_FOUND)
                continue;
            if (es_r == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_if = ps.eval_if();
            std::cout << "if: " << (int) es_if << std::endl;
            if (es_if == asc::STATE_FOUND)
                continue;
            if (es_r == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fc = ps.eval_function_call();
            std::cout << "function call: " << (int) es_fc << std::endl;
            if (es_fc == asc::STATE_FOUND)
            {
                ps.current = ps.current->next; // move past semicolon, only here because this is a standalone function call
                continue;
            }
            if (es_fc == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::err("unknown statement", ps.current->line);
            return -1;
        }
        /*
        std::cout << "symbols: " << std::endl;
        for (std::pair<std::string, asc::symbol*> pair : ps.symbols)
        {
            if (pair.second == nullptr)
                continue;
            std::cout << pair.second->visibility << ' ' << pair.second->type << ' ' << pair.second->name() << " in scope " << (pair.second->scope != nullptr ? pair.second->scope->name() : "global") << std::endl;
        }
        */
        asc::symbol*& entry = ps.symbols[ps.as.entry];
        if (entry == nullptr)
        {
            asc::err("no entry point found in program");
            return -1;
        }
        std::ofstream os = std::ofstream(filepath.substr(0, filepath.length() - 3) + ".asm", std::ios::trunc);
        std::string constructed = ps.as.construct();
        os.write(constructed.c_str(), constructed.length());
        os.close();
        is.close();
        return 0;
    }

    int visually_tokenize(std::string filepath)
    {
        if (!asc::ends_with(filepath, ".as"))
        {
            asc::err(filepath + " is not A# source code");
            return -1;
        }
        std::ifstream is = std::ifstream(filepath);
        asc::tokenizer tk = asc::tokenizer(is);
        for (std::string token; tk.extractable();)
            tk >> token;
        std::cout << filepath << " tokenized: " << std::endl;
        for (asc::syntax_node* current = tk.syntax_start(); current != nullptr; current = current->next)
            std::cout << current->stringify() << std::endl;
        is.close();
        return 0;
    }
}