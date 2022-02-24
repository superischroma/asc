#include <iostream>

#include "cli.h"
#include "logger.h"
#include "util.h"
#include "tokenizer.h"
#include "parser.h"

std::string SRC_ASSEMBLER = "nasm";
std::string SRC_LINKER = "gcc";
std::vector<std::string> OBJECT_FILES;
namespace asc
{
    asc::arg_result args;
}

int main(int argc, char* argv[])
{
    // init keyword regex pattern
    asc::KEYWORD_REGEX_PATTERN += "\\b(";
    for (auto it = asc::STANDARD_KEYWORDS.cbegin(); it != asc::STANDARD_KEYWORDS.cend(); it++)
        asc::KEYWORD_REGEX_PATTERN += (it != asc::STANDARD_KEYWORDS.cbegin() ? "|" : "") + *it;
    asc::KEYWORD_REGEX_PATTERN += ")\\b";

    // init tokenizer regex pattern
    asc::TOKENIZER_REGEX_PATTERN += '(';
    // keywords
    for (auto it = asc::STANDARD_KEYWORDS.cbegin(); it != asc::STANDARD_KEYWORDS.cend(); it++)
        asc::TOKENIZER_REGEX_PATTERN += (asc::TOKENIZER_REGEX_PATTERN.length() != 1 ? "|" : "") + *it;
    asc::TOKENIZER_REGEX_PATTERN += (asc::TOKENIZER_REGEX_PATTERN.length() != 1 ? "|" : "") + 
        std::string("\\\"[^\\\"\\\\\\\\]*(\\\\\\\\.[^\\\"\\\\\\\\]*)*\\\""); // string pattern
    asc::TOKENIZER_REGEX_PATTERN += "|([-.0-9u|U|l|L|f|F|D]+)"; // number pattern
    for (auto it = asc::STANDARD_PUNCTUATORS.cbegin(); it != asc::STANDARD_PUNCTUATORS.cend(); it++)
    {
        std::string punctuator = *it;
        asc::TOKENIZER_REGEX_PATTERN += '|' + asc::escape_chars_regex(punctuator);
    }
    asc::TOKENIZER_REGEX_PATTERN += "|[0-z]+";
    asc::TOKENIZER_REGEX_PATTERN += ')';
    std::cout << asc::TOKENIZER_REGEX_PATTERN << std::endl;

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
            asc::evaluation_state es_rs = ps.eval_return_statement();
            asc::debug("return statement: " + std::to_string((int) es_rs));
            if (es_rs == asc::STATE_FOUND)
                continue;
            if (es_rs == asc::STATE_SYNTAX_ERROR)
                return -1;
            asc::evaluation_state es_fd = ps.eval_function_header();
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
            asc::evaluation_state es_exp = ps.eval_expression();
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

    int experimental_compile(std::string& filepath)
    {
        return stable_compile(filepath);
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
        asc::syntax_node* current = asc::tokenize(is);
        asc::info(filepath + " tokenized: ");
        for (; current != nullptr; current = current->next)
            std::cout << current->stringify() << std::endl;
        is.close();
        return 0;
    }
}