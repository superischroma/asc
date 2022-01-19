use printf;

public int add(int x, int y)
{
    return x + y;
}

public int main()
{
    int x = 5;
    int y = 10;
    int z = add(x, y);
    printf("%d", z);
}