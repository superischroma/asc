use int printf(char*, int);

public object Location
{
    private int x;
    private int y;
    private int z;

    public constructor(int a, int b, int c)
    {
        this.x = a;
        this.y = b;
        this.z = c;
    }

    public void setX(int nx)
    {
        this.x = nx;
    }

    public int getX()
    {
        return this.x;
    }
}