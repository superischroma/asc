#ifndef CLI_H
#define CLI_H

#include <string>
#include <vector>

namespace asc
{
    namespace cli_options
    {
        const unsigned long long TOKENIZE = 1 << 0;
        const unsigned long long HELP = 1 << 1;
    }

    typedef struct s_arg_result
    {
        std::vector<std::string> files;
        unsigned long long options;
    } arg_result;

    typedef struct s_help_reference
    {
        std::string name;
        std::string description;
    } help_reference;

    extern help_reference REFERENCE_OPTIONS[];

    arg_result eval_args(int argc, char**& argv);
    bool has_option_set(arg_result& as, unsigned long long option);
}

#endif