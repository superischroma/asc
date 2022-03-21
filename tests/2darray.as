use int printf(char*, double);

int main()
{
    double** m ~= 2; # allocate array of 2 double arrays
    m[0] ~= 1; # allocate inner arrays
    m[1] ~= 1;
    m[0][0] = 5.3;
    m[1][1] = 10.6;
    m[1][0] = 18.3;
    printf("%f, ", m[0][0]);
    printf("%f, ", m[1][1]);
    printf("%f", m[1][0]);
}