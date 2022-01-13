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
        const unsigned long long SYMBOLIZE = 1 << 2;
    }

    typedef struct arg_result
    {
        std::vector<std::string> files;
        unsigned long long options;
        std::string output_location;
    } arg_result;

    typedef struct help_reference
    {
        std::string name;
        std::string description;
    } help_reference;

    extern std::vector<help_reference> REFERENCE_OPTIONS;

    arg_result eval_args(int argc, char**& argv);
    bool has_option_set(arg_result& as, unsigned long long option);
}

#endif