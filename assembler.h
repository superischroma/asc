#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <stdexcept>
#include <queue>
#include <deque>
#include <array>

namespace asc
{
    typedef std::string register_resolvable;

    extern std::vector<std::string> ARG_REGISTER_SEQUENCE;
    extern std::vector<std::string> FP_ARG_REGISTER_SEQUENCE;

    register_resolvable resolve_register(register_resolvable& identifier, int size);
    register_resolvable resolve_register(register_resolvable&& identifier, int size);

    class subroutine
    {
    public:
        std::string name;
        std::string instructions;
        //int stackalloc;
        int preserved_data;
        std::string ending;
        subroutine* parent;
        std::queue<std::string> data_queue;
        std::vector<subroutine*>* children;

        subroutine(std::string name, subroutine* parent);
        //subroutine& alloc_delta(int bs);
        subroutine& add_child(subroutine* sr);
        std::string construct();
        ~subroutine();
    };

    class assembler
    {
    private:
        std::string data;
        std::string bss;
        std::set<std::string> ext;
        std::map<std::string, subroutine*> subroutines;
    public:
        std::string entry;
        unsigned char write_mode;

        assembler(std::string entry = "main");
        assembler& enter(std::string& subroutine);
        assembler& enter(std::string&& subroutine);
        assembler& instruct(std::string& subroutine, std::string instruction);
        assembler& instruct(std::string&& subroutine, std::string instruction);
        assembler& queue_instruction(std::string& subroutine, std::string instruction);
        assembler& queue_instruction(std::string&& subroutine, std::string instruction);
        assembler& release(std::string& subroutine);
        assembler& release(std::string&& subroutine);
        assembler& external(std::string identifier);
        asc::subroutine*& sr(std::string& name, subroutine* parent);
        asc::subroutine*& sr(std::string& name);
        asc::subroutine*& sr(std::string&& name);
        assembler& operator<<(std::string& line);
        assembler& operator<<(std::string&& line);
        assembler& operator<<(assembler& (*mod)(assembler& as));
        std::string construct();
    };

    assembler& data(assembler& as);
    assembler& bss(assembler& as);
}

#endif