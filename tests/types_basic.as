use int printf(char*, double, double, double);

public type Vector
{
    float a;
    float b;
    float c;
}

public int main()
{
    Vector* vec = Vector(0.3f, 5.6f, 3.5f);
    printf("%f, %f, %f", vec.a => double, vec.b => double, vec.c => double);
    delete vec;
}