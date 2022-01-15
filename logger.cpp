#include <iostream>

#include "asc.h"

namespace asc
{
    void log(std::string& str, std::string&& descriptor)
    {
        std::cout << "asc: " << descriptor << ": " << str << std::endl;
    }

    void gen(std::string& str)
    {
        std::cout << "asc: " << str << std::endl;
    }

    void gen(std::string&& str)
    {
        gen(str);
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

    void debug(std::string& str)
    {
        if (asc::has_option_set(asc::args, asc::cli_options::DEBUG))
            log(str, "debug");
    }

    void debug(std::string&& str)
    {
        debug(str);
    }
}