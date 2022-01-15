use printf;

public type Point
{
    int x;
    int y;
}

public int main()
{
    int x = 10;
    if (x)
    {
        int x = 3;
        printf("%d", x);
    }
    printf("%d", x);
}