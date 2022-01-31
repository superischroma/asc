use printf;

public int t(int a, int b, int c, int d, int e, int z)
{
    printf("%d ", a);
    printf("%d ", b);
    printf("%d ", c);
    printf("%d ", d);
    printf("%d ", e);
    printf("%d ", z);
    return a + b + c + d + e + z;
}

public int v(int f, int g, int h, int i, int j, int k)
{
    return f + g + h + i + j + k;
}

public int main()
{
    t(2, 4, 6, 8, 10, 12);
    #printf("%d", t(5 + 6, (5 + 8) + 3, 6 + 3, 10 + 11, 100 + v(3 + 6, 10 + 20, 50 + 33, 100 + 68, 200 + 98, 6 + 3)));
}