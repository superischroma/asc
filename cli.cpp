#include "cli.h"

namespace asc
{
    help_reference REFERENCE_OPTIONS[] = {
        {"--help", "Shows this menu"},
        {"-tokenize", "Tokenizes the input file and displays it"}
    };

    arg_result eval_args(int argc, char**& argv)
    {
        arg_result as;
        as.options = 0;
        for (int i = 1; i < argc; i++)
        {
            std::string arg = std::string(argv[i]);
            if (arg == "-tokenize")
                as.options |= cli_options::TOKENIZE;
            else if (arg == "--help")
                as.options |= cli_options::HELP;
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