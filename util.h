#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>
#include <fstream>

#include "syntax.h"

namespace asc
{
    int strlen(const char* str);
    bool ends_with(std::string& base, std::string& test);
    bool ends_with(std::string& base, std::string&& test);
    std::map<std::string, std::string> map_cfg_file(std::ifstream& is);
    std::string to_lowercase(std::string str);
    std::string to_uppercase(std::string str);
    std::string stringify(std::deque<asc::rpn_element>& dq);
    std::string relative_dereference(std::string& relative_to, int offset);
    std::string relative_dereference(std::string&& relative_to, int offset);
    std::string escape_chars_regex(std::string& str);
    std::string escape_chars_regex(std::string&& str);
}

#endif