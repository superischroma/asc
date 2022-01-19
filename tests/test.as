use printf;

public int t(int a, int b, int c, int d, int e)
{
    return a + b + c + d + e;
}

public int main()
{
    t(5 + 6, (5 + 8) + 3, 6 + 3, 10 + 11, 100 + t(3 + 6, 10 + 20, 50 + 33, 100 + 68, 200 + 98));
}