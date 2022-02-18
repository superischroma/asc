use int printf(char[], byte);

byte test(byte a, short b, int c, long d)
{
    printf("%d, ", a);
    return 10;
}

int main()
{
    byte e = test(5, 3, 10, 15);
    printf("%d", e);
}