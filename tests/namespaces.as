use int printf(char*, long real);

namespace Constants
{
    long real PI = 3.14159265;
    namespace Special
    {
        long real E = 2.718;
    }
}

public int main()
{
    printf("%f, ", Constants::PI);
    printf("%f", Constants::Special::E);
}