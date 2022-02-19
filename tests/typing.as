use int printf(char[], byte);

byte test(byte a, short b, int c, long d)
{
    printf("%d, ", a);
    printf("%d, ", b);
    return 10;
}

int main()
{
    byte e = test(100 / 5 / 5, 11 % 5, 10, 15);
    printf("%d", e);
}