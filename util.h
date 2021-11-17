#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>
#include <fstream>

namespace asc
{
    int strlen(const char* str);
    bool ends_with(std::string& base, std::string& test);
    bool ends_with(std::string& base, std::string&& test);
    std::map<std::string, std::string> map_cfg_file(std::ifstream& is);
}

#endif