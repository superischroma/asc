#ifndef ASC_H
#define ASC_H

#include "cli.h"

namespace asc
{
    extern asc::arg_result args;

    int compile(std::string filepath);
    int visually_tokenize(std::string filepath);
}

#endif