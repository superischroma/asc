public int m2(int x)
{
    return x + x;
}

public int main()
{
    int i = m2(m2(2));
    while (i)
    {
        @print i;
        i = i - 1;
    }
}