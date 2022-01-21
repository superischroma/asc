use printf;

public int t(int a, int b, int c, int d, int e)
{
    return a + b + c + d + e;
}

public int v(int f, int g, int h, int i, int j)
{
    return f + g + h + i + j;
}

public int main()
{
    t(5 + 6, (5 + 8) + 3, 6 + 3, 10 + 11, 100 + v(3 + 6, 10 + 20, 50 + 33, 100 + 68, 200 + 98));
}