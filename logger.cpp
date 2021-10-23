#include <iostream>

namespace asc
{
    void log(std::string& str, std::string&& descriptor)
    {
        std::cout << "asc: " << descriptor << ": " << str << std::endl;
    }

    void info(std::string& str)
    {
        log(str, "info");
    }

    void info(std::string&& str)
    {
        info(str);
    }

    void warn(std::string& str)
    {
        log(str, "warn");
    }

    void warn(std::string&& str)
    {
        warn(str);
    }

    void err(std::string& str)
    {
        log(str, "error");
    }

    void err(std::string&& str)
    {
        err(str);
    }

    void err(std::string& str, int line)
    {
        log(str, "error [" + std::to_string(line) + ']');
    }

    void err(std::string&& str, int line)
    {
        err(str, line);
    }
}