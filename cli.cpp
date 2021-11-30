#include "cli.h"
#include "logger.h"

namespace asc
{
    std::vector<help_reference> REFERENCE_OPTIONS {
        {"--help", "Shows this menu"},
        {"-tokenize", "Tokenizes the input file and displays it"},
        {"-o <location>", "Specifies an output location"}
    };

    arg_result eval_args(int argc, char**& argv)
    {
        arg_result as;
        as.output_location = "a";
        as.options = 0;
        for (int i = 1; i < argc; i++)
        {
            std::string arg = std::string(argv[i]);
            if (arg == "-tokenize")
                as.options |= cli_options::TOKENIZE;
            else if (arg == "--help")
                as.options |= cli_options::HELP;
            else if (arg == "-o")
            {
                arg = std::string(argv[++i]);
                if (i >= argc)
                    asc::warn("output location not specified, using default");
                else
                    as.output_location = arg;
            }
            else // file
                as.files.push_back(arg);
        }
        return as;
    }
    
    bool has_option_set(arg_result& as, unsigned long long option)
    {
        return (as.options & option) != 0;
    }
}