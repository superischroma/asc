########################
#       POINTERS       #
########################

private void pointerTest()
{
    int i = 5;
    int* ip = &i; # Gets the memory address of i
    int it = *ip; # Retrieve value of memory address pointer ip
}

#####################
#       TYPES       #
#####################

public type Point { # type <identifier> {...}
    int x;
    int y = 0; # default initializer
}

# Implementation

private Point bottom()
{
    return Point(0, -10); # Implicit constructor (x, then y, etc...)
}

private Point bottomExplicit()
{
    return Point(x = 0, y = -10); # Explicit constructor (random order, prefixed by <variable> =)
}

# These two constructor types CANNOT be mixed.

# Member access
private int getY(Point p)
{
    return p.y; # Dot (.) operator used to access value
}

# Member modification
private void setY(Point* p, int ny)
{
    (*p).y = ny; # Dereferencing p and setting y to ny
}

#######################
#       OBJECTS       #
#######################

public object Location
{
    # Instance variables
    private int x;
    private int y;
    private int z;

    # Object constructors
    public constructor(int cx, int cy, int cz)
    {
        x = cx;
        y = cy;
        z = cz;
    }

    # Default constructor
    public constructor()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    # Mutator methods
    public int setX(int nx)
    {
        int ox = x;
        x = nx;
        return ox;
    }

    public const Location zero() # Constant method; cannot be overridden in subclasses
    {
        x = 0;
        y = 0;
        z = 0;
        return *this;
    }

    public Location +=(int d) # Overloaded operator method for +=
    {
        x += d;
        y += d;
        z += d;
        return *this;
    }

    public segregate Location zeroed() # Segregate methods, not suggested to be used
    {
        return Location(0, 0, 0);
    }
}

#################################
#       CONCEPTUAL OBJECTS      #
#################################

public object Animal
{
    public conceptual void speak(); # Conceptual method
    # This method declaration marks Animal as a conceptual object
}

#################################
#       BASIC INHERITANCE       #
#################################

public object Dog extends Animal
{
    public void speak() # Implemented from Animal
    {
        # Print "woof" here
    }
}

#############################
#       GENERIC TYPES       #
#############################

public object Container<E>
{
    private E value;

    public constructor(E v)
    {
        value = v;
    }

    public constructor() {}

    public E getValue()
    {
        return value;
    }
}

private Location locationHorizontal(int vx, int vz)
{
    return Location(vx, 100, vz); # Explicit, like types
}

private Location locationHorizontalExplicit(int vx, int vz)
{
    return Location(x = vx, y = 100, z = vz)
}