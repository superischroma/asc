use printf;

byte test(byte a, short b, int c, long d)
{
    printf("%d", a);
}

int main()
{
    test(5, 3, 10, 15);
}