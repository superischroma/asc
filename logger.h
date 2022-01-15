#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace asc
{
    void info(std::string& str);
    void info(std::string&& str);
    void gen(std::string& str);
    void gen(std::string&& str);
    void warn(std::string& str);
    void warn(std::string&& str);
    void err(std::string& str);
    void err(std::string&& str);
    void err(std::string& str, int line);
    void err(std::string&& str, int line);
    void debug(std::string& str);
    void debug(std::string&& str);
}

#endif