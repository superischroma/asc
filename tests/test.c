int t(int a, int b, int c, int d, int e, int z)
{
    return a + b + c + d + e + z;
}

int v(int f, int g, int h, int i, int j, int k)
{
    return f + g + h + i + j + k;
}

int main()
{
    int u = 5;
    t(2ull, 4ull, 6ull, 8ull, 10, 12);
    //printf("%d", t(5 + 6, (5 + 8) + 3, 6 + 3, 10 + 11, 100 + v(3 + 6, 10 + 20, 50 + 33, 100 + 68, 200 + 98, 6 + 3)));
}