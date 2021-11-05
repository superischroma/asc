int str(char* s)
{
    int j = *(s + 1) + 5;
    return j;
}

int main()
{
    str("test");
}