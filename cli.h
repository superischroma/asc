#ifndef CLI_H
#define CLI_H

#include <string>
#include <vector>

namespace asc
{
    namespace cli_options
    {
        const unsigned long long TOKENIZE = 1 << 0;
    }

    typedef struct s_arg_result
    {
        std::vector<std::string> files;
        unsigned long long options;
    } arg_result;

    arg_result eval_args(int argc, char**& argv);
    bool has_option_set(arg_result& as, unsigned long long option);
}

#endif