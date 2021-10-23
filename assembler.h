#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <map>
#include <stdexcept>

namespace asc
{
    class assembler
    {
    private:
        std::string* entry;
        std::string data;
        std::string bss;
        std::map<std::string, std::string> subroutines;
    public:
        unsigned char write_mode;

        assembler(std::string&& entry = "main")
        {
            this->entry = &entry;
            this->write_mode = 0;
        }

        assembler& enter(std::string& subroutine)
        {
            this->entry = &subroutine;
            return *this;
        }

        assembler& enter(std::string&& subroutine)
        {
            return enter(subroutine);
        }

        assembler& instruct(std::string& subroutine, std::string instruction)
        {
            subroutines[subroutine] += "\n\t" + instruction;
            return *this;
        }

        assembler& instruct(std::string&& subroutine, std::string instruction)
        {
            instruct(subroutine, instruction);
            return *this;
        }

        assembler& operator<<(std::string& line)
        {
            switch (write_mode)
            {
                case 0:
                {
                    if (data.length() != 0) data += '\n';
                    data += line;
                    break;
                }
                case 1:
                {
                    if (data.length() != 0) bss += '\n';
                    bss += line;
                    break;
                }
                default:
                    throw std::runtime_error("invalid write mode for assembler");
            }
            return *this;
        }

        assembler& operator<<(std::string&& line)
        {
            return operator<<(line);
        }

        assembler& operator<<(assembler& (*mod)(assembler& as))
        {
            return mod(*this);
        }

        std::string construct()
        {
            std::string f;
            if (data.length() != 0)
                f += "section .data" + data;
            if (bss.length() != 0)
            {
                if (data.length() != 0) f += '\n';
                f += "section .bss" + bss;
            }
            if (subroutines.size() == 0)
                return f;
            if (data.length() != 0 || bss.length() != 0)
                f += '\n';
            f += "section .text";
            f += "\nglobal " + *entry;
            for (std::pair<std::string, std::string> subroutine : subroutines)
                f += '\n' + subroutine.first + ':' + subroutine.second;
            return f;
        }
    };

    assembler& data(assembler& as)
    {
        as.write_mode = 0;
        return as;
    }

    assembler& bss(assembler& as)
    {
        as.write_mode = 1;
        return as;
    }
}

#endif