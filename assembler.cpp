#include "assembler.h"
#include "logger.h"

namespace asc
{
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
        if (identifier.find("[rbp") != std::string::npos) // it's not a register lol
            return "-1";
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
        if (identifier.find("si") != std::string::npos)
            return REGISTER_TABLE[16 + lindex];
        if (identifier.find("di") != std::string::npos)
            return REGISTER_TABLE[20 + lindex];
        if (identifier.find("bp") != std::string::npos)
            return REGISTER_TABLE[24 + lindex];
        if (identifier.find("sp") != std::string::npos)
            return REGISTER_TABLE[28 + lindex];
        if (identifier.find('a') != std::string::npos)
            return REGISTER_TABLE[0 + lindex];
        if (identifier.find('b') != std::string::npos)
            return REGISTER_TABLE[4 + lindex];
        if (identifier.find('c') != std::string::npos)
            return REGISTER_TABLE[8 + lindex];
        if (identifier.find('d') != std::string::npos)
            return REGISTER_TABLE[12 + lindex];
        return "-1";
    }

    register_resolvable resolve_register(register_resolvable&& identifier, int size)
    {
        return resolve_register(identifier, size);
    }

    subroutine::subroutine(std::string name, subroutine* parent)
    {
        this->name = name;
        this->preserved_data = 0;
        this->ending = "ret";
        this->parent = parent;
        this->children = nullptr;
        if (parent != nullptr)
            parent->add_child(this);
    }

    /*subroutine& subroutine::alloc_delta(int bs)
    {
        if (this->parent == nullptr)
            this->stackalloc += bs;
        else
            this->parent->stackalloc += bs;
        return *this;
    }*/

    subroutine& subroutine::add_child(subroutine* sr)
    {
        if (children == nullptr)
            children = new std::vector<subroutine*>();
        children->push_back(sr);
        return *this;
    }

    std::string subroutine::construct()
    {
        std::string str;
        int space;
        if (this->parent == nullptr)
        {
            asc::debug("preserved for " + name + ": " + std::to_string(preserved_data));
            space = 40 /* shadow space */ + preserved_data;
            space += (space % 16); // 16-byte alignment for calling convention
            asc::debug("space: " + std::to_string(space));
            str += "\n\tpush rbp\n\tmov rbp, rsp";
            if (space != 0)
                str += "\n\tsub rsp, " + std::to_string(space);
        }
        str += instructions;
        if ((this->parent != nullptr &&
            this->parent->children != nullptr &&
            this->parent->children->size() > 0 &&
            this->parent->children->at(this->parent->children->size() - 1)->name == this->name)
            ||
            (this->parent == nullptr &&
                (this->children == nullptr ||
                    (this->children != nullptr && this->children->size() == 0))))
        {
            asc::debug("preserved for " + name + ": " + std::to_string(preserved_data));
            space = 40 /* shadow space */ + (parent != nullptr ? parent->preserved_data : preserved_data);
            space += (space % 16); // 16-byte alignment for calling convention
            if (this->parent != nullptr || space != 0)
                str += "\n\tadd rsp, " + std::to_string(space);
            str += "\n\tpop rbp";
        }
        if (ending.length() != 0)
            str += "\n\t" + ending;
        return str;
    }

    subroutine::~subroutine()
    {
        if (children != nullptr) delete children;
    }

    assembler::assembler(std::string entry)
    {
        this->entry = entry;
        this->write_mode = 0;
        this->ext = std::set<std::string>();
    }

    assembler& assembler::enter(std::string& subroutine)
    {
        this->entry = subroutine;
        return *this;
    }

    assembler& assembler::enter(std::string&& subroutine)
    {
        return enter(subroutine);
    }

    assembler& assembler::instruct(std::string& subroutine, std::string instruction)
    {
        asc::subroutine*& sr = subroutines[subroutine];
        if (sr == nullptr)
            sr = new asc::subroutine(subroutine, nullptr);
        sr->instructions += "\n\t" + instruction;
        asc::debug("added \"" + instruction + "\" to " + subroutine);
        return *this;
    }

    assembler& assembler::instruct(std::string&& subroutine, std::string instruction)
    {
        return instruct(subroutine, instruction);
    }

    assembler& assembler::queue_instruction(std::string& subroutine, std::string instruction)
    {
        asc::subroutine*& sr = subroutines[subroutine];
        if (sr == nullptr)
            sr = new asc::subroutine(subroutine, nullptr);
        sr->data_queue.push(instruction);
        return *this;
    }

    assembler& assembler::queue_instruction(std::string&& subroutine, std::string instruction)
    {
        return queue_instruction(subroutine, instruction);
    }

    assembler& assembler::release(std::string& subroutine)
    {
        asc::subroutine*& sr = subroutines[subroutine];
        if (sr != nullptr && !sr->data_queue.empty())
            instruct(subroutine, sr->data_queue.front());
        return *this;
    }

    assembler& assembler::release(std::string&& subroutine)
    {
        return release(subroutine);
    }

    assembler& assembler::external(std::string identifier)
    {
        ext.insert(identifier);
        return *this;
    }

    asc::subroutine*& assembler::sr(std::string& name, subroutine* parent)
    {
        asc::subroutine*& sr = subroutines[name];
        if (sr == nullptr)
            sr = new asc::subroutine(name, parent);
        return sr;
    }

    asc::subroutine*& assembler::sr(std::string& name)
    {
        asc::subroutine*& sr = subroutines[name];
        if (sr == nullptr)
            sr = new asc::subroutine(name, nullptr);
        return sr;
    }

    asc::subroutine*& assembler::sr(std::string&& name)
    {
        return sr(name);
    }

    assembler& assembler::operator<<(std::string& line)
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

    assembler& assembler::operator<<(std::string&& line)
    {
        return operator<<(line);
    }

    assembler& assembler::operator<<(assembler& (*mod)(assembler& as))
    {
        return mod(*this);
    }

    std::string assembler::construct()
    {
        std::string f;
        for (auto& e : ext)
            f += "extern " + e + '\n';
        if (data.length() != 0)
            f += "section .data" + data;
        if (bss.length() != 0)
        {
            if (data.length() != 0) f += '\n';
            f += "section .bss" + bss;
        }
        if (subroutines.size() == 0)
            return f;
        if (ext.size() != 0 || data.length() != 0 || bss.length() != 0)
            f += '\n';
        f += "section .text";
        f += "\nglobal " + entry;
        for (std::pair<std::string, asc::subroutine*> subroutine : subroutines)
            f += '\n' + subroutine.first + ':' + subroutine.second->construct();
        return f;
    }

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