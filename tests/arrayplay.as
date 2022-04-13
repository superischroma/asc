use int printf(char*, int);

void out(int* arr, int index)
{
    printf("%d", arr[index]);
}

public int main()
{
    int* test ~= 5;
    test[0] = 3;
    out(test, 0);
    delete test;
}