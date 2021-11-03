#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace asc
{
    int strlen(const char* str);
    bool ends_with(std::string& base, std::string& test);
    bool ends_with(std::string& base, std::string&& test);
}

#endif