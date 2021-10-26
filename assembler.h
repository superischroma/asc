#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <map>
#include <stdexcept>

namespace asc
{
    typedef std::string register_resolvable;

    const char* REGISTER_TABLE[] = {
        "rax", "eax", "ax", "al",
        "rbx", "ebx", "bx", "bl", 
        "rcx", "ecx", "cx", "cl", 
        "rdx", "edx", "dx", "dl", 
        "rsi", "esi", "si", "sil", 
        "rdi", "edi", "di", "dil", 
        "rbp", "ebp", "bp", "bpl", 
        "rsp", "esp", "sp", "spl", 
        "r8", "r8d", "r8w", "r8b", 
        "r9", "r9d", "r9w", "r9b", 
        "r10", "r10d", "r10w", "r10b", 
        "r11", "r11d", "r11w", "r11b", 
        "r12", "r12d", "r12w", "r12b", 
        "r13", "r13d", "r13w", "r13b", 
        "r14", "r14d", "r14w", "r14b", 
        "r15", "r15d", "r15w", "r15b"
    };
    
    const char* ARG_REGISTER_SEQUENCE[] = {
        "rcx", "rdx", "r8", "r9"
    };

    register_resolvable resolve_register(register_resolvable& identifier, int size)
    {
        int lindex = -1;
        if (size == 1)
            lindex = 3;
        else if (size == 2)
            lindex = 2;
        else if (size == 4)
            lindex = 1;
        else if (size == 8)
            lindex = 0;
        else
            return "-1";
        if (identifier.find('a') != std::string::npos)
            return REGISTER_TABLE[0 + lindex];
        if (identifier.find('b') != std::string::npos)
            return REGISTER_TABLE[4 + lindex];
        if (identifier.find('c') != std::string::npos)
            return REGISTER_TABLE[8 + lindex];
        if (identifier.find('d') != std::string::npos)
            return REGISTER_TABLE[12 + lindex];
        if (identifier.find("si") != std::string::npos)
            return REGISTER_TABLE[16 + lindex];
        if (identifier.find("di") != std::string::npos)
            return REGISTER_TABLE[20 + lindex];
        if (identifier.find("bp") != std::string::npos)
            return REGISTER_TABLE[24 + lindex];
        if (identifier.find("sp") != std::string::npos)
            return REGISTER_TABLE[28 + lindex];
        if (identifier.find("r8") != std::string::npos)
            return REGISTER_TABLE[32 + lindex];
        if (identifier.find("r9") != std::string::npos)
            return REGISTER_TABLE[36 + lindex];
        if (identifier.find("r10") != std::string::npos)
            return REGISTER_TABLE[40 + lindex];
        if (identifier.find("r11") != std::string::npos)
            return REGISTER_TABLE[44 + lindex];
        if (identifier.find("r12") != std::string::npos)
            return REGISTER_TABLE[48 + lindex];
        if (identifier.find("r13") != std::string::npos)
            return REGISTER_TABLE[52 + lindex];
        if (identifier.find("r14") != std::string::npos)
            return REGISTER_TABLE[56 + lindex];
        if (identifier.find("r15") != std::string::npos)
            return REGISTER_TABLE[60 + lindex];
        return "-1";
    }

    register_resolvable resolve_register(register_resolvable&& identifier, int size)
    {
        return resolve_register(identifier, size);
    }

    class subroutine
    {
    public:
        bool functional;
        std::string instructions;
        int stackalloc;
        int next_local_offset;
        subroutine(bool functional = true)
        {
            this->functional = functional;
            this->stackalloc = 0;
            this->next_local_offset = 0;
        }

        std::string construct()
        {
            std::string str;
            if (this->functional)
            {
                str += "\n\tpush rbp\n\tmov rbp, rsp";
                if (this->stackalloc != 0)
                    str += "\n\tsub rsp, " + std::to_string(this->stackalloc);
            }
            str += instructions;
            if (this->functional)
            {
                if (this->stackalloc != 0)
                    str += "\n\tadd rsp, " + std::to_string(this->stackalloc);
                str += "\n\tpop rbp\n\tret";
            }
            return str;
        }
    };

    class assembler
    {
    private:
        std::string data;
        std::string bss;
        std::map<std::string, subroutine*> subroutines;
    public:
        std::string entry;
        unsigned char write_mode;

        assembler(std::string entry = "main")
        {
            this->entry = entry;
            this->write_mode = 0;
        }

        assembler& enter(std::string& subroutine)
        {
            this->entry = subroutine;
            return *this;
        }

        assembler& enter(std::string&& subroutine)
        {
            return enter(subroutine);
        }

        assembler& instruct(std::string& subroutine, std::string instruction, bool functional = true)
        {
            asc::subroutine*& sr = subroutines[subroutine];
            if (sr == nullptr)
                sr = new asc::subroutine(functional);
            sr->instructions += "\n\t" + instruction;
            return *this;
        }

        assembler& instruct(std::string&& subroutine, std::string instruction, bool functional = true)
        {
            return instruct(subroutine, instruction, functional);
        }

        assembler& alloc(std::string& subroutine, int bs, bool functional = true)
        {
            asc::subroutine*& sr = subroutines[subroutine];
            if (sr == nullptr)
                sr = new asc::subroutine(functional);
            sr->stackalloc = bs;
            return *this;
        }

        assembler& alloc_delta(std::string& subroutine, int bs, bool functional = true)
        {
            asc::subroutine*& sr = subroutines[subroutine];
            if (sr == nullptr)
                sr = new asc::subroutine(functional);
            sr->stackalloc += bs;
            return *this;
        }

        int allocated(std::string& subroutine)
        {
            asc::subroutine*& sr = subroutines[subroutine];
            if (sr == nullptr)
                return -1;
            return sr->stackalloc;
        }

        int offset(std::string& subroutine, bool functional = true)
        {
            asc::subroutine*& sr = subroutines[subroutine];
            if (sr == nullptr)
                sr = new asc::subroutine(functional);
            return sr->next_local_offset -= 8;
        }

        assembler& operator<<(std::string& line)
        {
            switch (write_mode)
            {
                case 0:
                {
                    data += '\n' + line;
                    break;
                }
                case 1:
                {
                    bss += '\n' + line;
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
            f += "\nglobal " + entry;
            for (std::pair<std::string, asc::subroutine*> subroutine : subroutines)
                f += '\n' + subroutine.first + ':' + subroutine.second->construct();
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