use int printf(char*, double);

public int main()
{
    # assign an integer to an array declaration to allocate x amount of elements
    #int* later ~= 10; # this creates an array of 10 ints
    #later[0] = 15;
    #later[1] = 30;
    #printf("%d, ", later[0]);
    #printf("%d", later[1]);
    double* numbers ~= 5;
    numbers[0] = 3.5;
    numbers[1] = 6.3;
    printf("%f, ", numbers[0]);
    printf("%f", numbers[1]);
    # assign an array literal to an array declaration
    #double* now = [5.3, 6.4];
    # obtain and print a value
    #printf("%f", now[0]); # result: 5.3
    # assign a new value to an array index
    #now[1] = 7.3; # 6.4 has been overridden by 7.3 at index 1
    #printf("%f", now[1]); # result: 7.3
}