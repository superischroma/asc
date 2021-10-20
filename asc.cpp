#include <iostream>

#include "syntax.h"

int main()
{
    std::string str = "57.4";
    asc::constant_node<int> node = asc::constant_node<int>(str);
    std::cout << node.value << std::endl;
}