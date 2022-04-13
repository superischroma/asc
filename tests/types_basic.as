use int printf(char*, long real, long real, long real);

public type Vector
{
    real a;
    real b;
    real c;
}

public int main()
{
    long real x = 5.3;
    Vector* vec = Vector(x => real, 5.6f, 3.5f);
    printf("%f, %f, %f", vec.a => long real, vec.b => long real, vec.c => long real);
    delete vec;
}