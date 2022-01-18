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

std::string SRC_ASSEMBLER = "nasm";
std::string SRC_LINKER = "gcc";
std::vector<std::string> OBJECT_FILES;
namespace asc
{
    asc::arg_result args;
}

int main(int argc, char* argv[])
{
    std::ifstream ois = std::ifstream("options.cfg");
    if (ois.fail())
        asc::warn("no options file found, using default options");
    else
    {
        auto options = asc::map_cfg_file(ois);
        try
        {
            SRC_ASSEMBLER = options.at("assembler");
        }
        catch (std::out_of_range e)
        {
            asc::warn("no assembler specified, using default assembler");
            SRC_ASSEMBLER = "nasm";
        }
        try
        {
            SRC_LINKER = options.at("linker");
        }
        catch (std::out_of_range e)
        {
            asc::warn("no linker specified, using default linker");
            SRC_LINKER = "gcc";
        }
    }
    asc::args = asc::eval_args(argc, argv);
    if (asc::has_option_set(asc::args, asc::cli_options::HELP))
    {
        std::cout << "Usage: asc [options] file..." << std::endl;
        std::cout << "Options:" << std::endl;
        for (int i = 0; i < asc::REFERENCE_OPTIONS.size(); i++)
        {
            asc::help_reference& hr = asc::REFERENCE_OPTIONS[i];
            std::cout << "  " << hr.name << "\t\t" << hr.description << std::endl;
        }
        return 0;
    }
    if (asc::args.files.size() <= 0)
    {
        asc::err("no input files");
        return -1;
    }
    if (asc::has_option_set(asc::args, asc::cli_options::TOKENIZE))
    {
        for (auto const& file : asc::args.files)
        {
            if (asc::visually_tokenize(file) == -1)
                return -1;
        }
        return 0;
    }
    for (auto const& file : asc::args.files)
    {
        if (asc::compile(file) == -1)
            return -1;
    }
    if (SRC_LINKER == "gcc" || SRC_LINKER == "ld")
    {
        std::string cmd = SRC_LINKER + " -o " + asc::args.output_location;
        for (auto& file : OBJECT_FILES)
            cmd += ' ' + file;
        system(cmd.c_str());
    }
    else
    {
        asc::err("linking with unsupported linker: " + SRC_LINKER);
        return -1;
    }
    asc::info("object code has been linked and executable has been created");
    return 0;
}
namespace asc
{
    int experimental_compile(std::string& filepath)
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
            asc::debug(current->stringify());
        asc::parser ps = asc::parser(tk.syntax_start());
        while (ps.parseable())
        {
            asc::debug("token: " + *(ps.current->value));
            asc::evaluation_state es_be = ps.eval_block_ending();
            asc::debug("block ending: " + std::to_string((int) es_be));
            if (es_be == asc::STATE_FOUND)
                continue;
            if (es_be == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_ue = ps.eval_use();
            asc::debug("use: " + std::to_string((int) es_ue));
            if (es_ue == asc::STATE_FOUND)
                continue;
            if (es_ue == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fd = ps.eval_function_decl();
            asc::debug("function decl: " + std::to_string((int) es_fd));
            if (es_fd == asc::STATE_FOUND)
                continue;
            if (es_fd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_vd = ps.eval_var_declaration();
            asc::debug("variable decl: " + std::to_string((int) es_vd));
            if (es_vd == asc::STATE_FOUND)
                continue;
            if (es_vd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_exp = ps.experimental_eval_expression();
            asc::debug("expression: " + std::to_string((int) es_exp));
            if (es_exp == asc::STATE_FOUND)
                continue;
            if (es_exp == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::err("unknown statement", ps.current->line);
            return -1;
        }
        asc::symbol* entry = ps.symbol_table_get(ps.as.entry);
        if (entry == nullptr)
        {
            asc::err("no entry point found in program");
            return -1;
        }
        std::string asmfn = filepath.substr(0, filepath.length() - 3) + ".asm";
        std::ofstream os = std::ofstream(asmfn, std::ios::trunc);
        std::string constructed = ps.as.construct();
        os.write(constructed.c_str(), constructed.length());
        os.close();
        is.close();
        asc::info("source code of \"" + filepath + "\" has been successfully converted to assembly");
        if (SRC_ASSEMBLER == "nasm")
            system(("nasm -fwin64 " + asmfn).c_str());
        else
        {
            asc::err("assembling \"" + filepath + "\" with unsupported assembler");
            return -1;
        }
        asc::info("assembly code of \"" + asmfn + "\" has been successfully converted to object code");
        OBJECT_FILES.push_back(filepath.substr(0, filepath.length() - 3) + ".obj");
        return 0;
    }
    
    int stable_compile(std::string& filepath)
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
            asc::debug(current->stringify());
        asc::parser ps = asc::parser(tk.syntax_start());
        while (ps.parseable())
        {
            asc::debug("token: " + *(ps.current->value));
            asc::evaluation_state es_ue = ps.eval_use();
            asc::debug("use: " + std::to_string((int) es_ue));
            if (es_ue == asc::STATE_FOUND)
                continue;
            if (es_ue == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_be = ps.eval_block_ending();
            asc::debug("block ending: " + std::to_string((int) es_be));
            if (es_be == asc::STATE_FOUND)
                continue;
            if (es_be == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fd = ps.eval_function_decl();
            asc::debug("function decl: " + std::to_string((int) es_fd));
            if (es_fd == asc::STATE_FOUND)
                continue;
            if (es_fd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_tc = ps.eval_type_construct();
            asc::debug("type creation: " + std::to_string((int) es_tc));
            if (es_tc == asc::STATE_FOUND)
                continue;
            if (es_tc == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_vdd = ps.eval_variable_decl_def();
            asc::debug("variable decl/def: " + std::to_string((int) es_vdd));
            if (es_vdd == asc::STATE_FOUND)
                continue;
            if (es_vdd == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_r = ps.eval_ret();
            asc::debug("return: " + std::to_string((int) es_r));
            if (es_r == asc::STATE_FOUND)
                continue;
            if (es_r == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_if = ps.eval_if();
            asc::debug("if: " + std::to_string((int) es_if));
            if (es_if == asc::STATE_FOUND)
                continue;
            if (es_if == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_wl = ps.eval_while();
            asc::debug("while: " + std::to_string((int) es_wl));
            if (es_wl == asc::STATE_FOUND)
                continue;
            if (es_wl == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fc = ps.eval_function_call();
            asc::debug("function call: " + std::to_string((int) es_fc));
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
        asc::symbol* entry = ps.symbol_table_get(ps.as.entry);
        if (entry == nullptr)
        {
            asc::err("no entry point found in program");
            return -1;
        }
        std::string asmfn = filepath.substr(0, filepath.length() - 3) + ".asm";
        std::ofstream os = std::ofstream(asmfn, std::ios::trunc);
        std::string constructed = ps.as.construct();
        os.write(constructed.c_str(), constructed.length());
        os.close();
        is.close();
        asc::info("source code of \"" + filepath + "\" has been successfully converted to assembly");
        if (SRC_ASSEMBLER == "nasm")
            system(("nasm -fwin64 " + asmfn).c_str());
        else
        {
            asc::err("assembling \"" + filepath + "\" with unsupported assembler");
            return -1;
        }
        asc::info("assembly code of \"" + asmfn + "\" has been successfully converted to object code");
        OBJECT_FILES.push_back(filepath.substr(0, filepath.length() - 3) + ".obj");
        return 0;
    }

    int compile(std::string filepath)
    {
        if (!has_option_set(args, cli_options::EXPERIMENTAL)) // if we're not in experimental mode
            return stable_compile(filepath);
        else
            return experimental_compile(filepath);
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
        asc::info(filepath + " tokenized: ");
        for (asc::syntax_node* current = tk.syntax_start(); current != nullptr; current = current->next)
            std::cout << current->stringify() << std::endl;
        is.close();
        return 0;
    }
}