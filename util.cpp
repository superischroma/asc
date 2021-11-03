#include <iostream>
#include <string>

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
}