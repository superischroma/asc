#include <stdio.h>

int nothing(int v)
{
    return v + v;
}

int main()
{
    int i = 328;
    printf("%d", nothing(i));
}