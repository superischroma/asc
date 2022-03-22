use int printf(char*, int);

public type Point
{
    int x;
    int y;
}

public int main()
{
    Point* point = Point(5, 3);
    point.x = 10;
    #printf("%d, ", point.x);
    #printf("%d", point.y);
    delete point;
}