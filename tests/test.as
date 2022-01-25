use printf;

public int t(int a, int b, int c, int d, int e)
{
    return a + b + c + d + e;
}

public int v(int f, int g, int h, int i, int j, int k)
{
    return f + g + h + i + j + k;
}

public int main()
{
    printf("%d", t(5 + 6, (5 + 8) + 3, 6 + 3, 10 + 11, 100 + v(3 + 6, 10 + 20, 50 + 33, 100 + 68, 200 + 98, 6 + 3)));
}