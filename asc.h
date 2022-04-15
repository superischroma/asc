#ifndef ASC_H
#define ASC_H

#include "cli.h"

namespace asc
{
    class parser;
    extern asc::arg_result args;

    int compile(std::string filepath, parser* m = nullptr);
    int visually_tokenize(std::string filepath);
    int analyze_expressions(std::string filepath);
}

#endif