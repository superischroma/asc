#include <iostream>

#include "util.h"

namespace asc
{
    int strlen(const char* str)
    {
        int length = 0;
        for (; str[length] != '\0'; length++);
        return length;
    }

    bool ends_with(std::string& base, std::string& test)
    {
        if (base.length() < test.length())
            return false;
        for (int i = base.length() - 1, j = test.length() - 1; j >= 0; i--, j--)
        {
            if (base[i] != test[j])
                return false;
        }
        return true;
    }

    bool ends_with(std::string& base, std::string&& test)
    {
        return ends_with(base, test);
    }

    std::map<std::string, std::string> map_cfg_file(std::ifstream& is)
    {
        std::map<std::string, std::string> map;
        std::string key, value;
        bool ek = true;
        bool comment = false;
        for (char c = is.get(); is.good(); c = is.get())
        {
            if (c == '\r')
                continue;
            if (c == '#' || c == ';')
            {
                comment = true;
                continue;
            }
            if (c == '\n')
            {
                if (key.length() != 0)
                    map[key] = value;
                key.clear();
                value.clear();
                comment = false;
                ek = true;
                continue;
            }
            if (comment)
                continue;
            if (c == '=')
            {
                ek = false;
                continue;
            }
            (ek ? key : value) += c;
        }
        if (key.length() != 0)
            map[key] = value;
        return map;
    }

    std::string to_lowercase(std::string str)
    {
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] >= 'a' && str[i] <= 'z')
                continue;
            str[i] += ' ';
        }
        return str;
    }

    std::string to_uppercase(std::string str)
    {
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] >= 'A' && str[i] <= 'Z')
                continue;
            str[i] -= ' ';
        }
        return str;
    }
}