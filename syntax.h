#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

#include <string>
#include <type_traits>

namespace asc
{
    enum syntax_types
    {
        CONSTANT = 0
    };

    class syntax_node
    {
    public:
        syntax_node* left;
        syntax_node* right;
    protected:
        unsigned short type;
        syntax_node(unsigned short type)
        {
            this->type = type;
        }
    };

    template <typename N>
    class constant_node: public syntax_node
    {
    public:
        N value;
        constant_node(std::string& rep): syntax_node(syntax_types::CONSTANT)
        {
            if (typeid(N) == typeid(char))
                value = static_cast<char>(std::stoi(rep));
            else if (typeid(N) == typeid(short))
                value = static_cast<short>(std::stoi(rep));
            else if (typeid(N) == typeid(int))
                value = std::stoi(rep);
            else if (typeid(N) == typeid(long) || typeid(N) == typeid(long long))
                value = std::stoll(rep);
            else if (typeid(N) == typeid(float))
                value = std::stof(rep);
            else if (typeid(N) == typeid(double) || typeid(N) == typeid(long double))
                value = std::stod(rep);
        }
    };
}

#endif