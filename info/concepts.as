#########################
#       CONSTANTS       #
#########################

# Any variable in global scope is considered a constant.
public const int PIES = 50; # UPPER_SNAKE_CASE public constant, made a constant explicitly
private long UNMARKED_CONSTANT = 7; # private constant, made a constant implicitly

#########################
#       ACCESSORS       #
#########################

public void accessorPublic()
{
    # This is a public function
    # This will be available in other files
}

private void accessorPrivate()
{
    # This is a private function
    # This cannot be accessed in other files
}

void accessorNone()
{
    # This has no accessor
    # It is set to private by default
    # 'void example() {...}' is the same as 'private void example() {...}'
}

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

public type Point # type <identifier> {...}
{
    int x;
    int y = 0; # default initializer
}

#################################
#       TYPE INHERITANCE        #
#################################

public type Point3D extends Point
{
    int z;
}

#############################
#       GENERIC TYPES       #
#############################

public type Data<T>
{
    T value;
}

#################################
#       GENERIC FUNCTIONS       #
#################################

public <T> T newGeneric()
{
    return T();
}

# Usage

public void genericUsage()
{
    int v = newGeneric<int>(); # Type is in diamond
}

#####################
#       CASTING     #
#####################

public int castExample(long l)
{
    return (int) l; # Casting to int
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
    p->y = ny; # Shorthand
}

#######################
#       OBJECTS       #
#######################

public object Location
{
    # Segregate variables; these are NOT made constant explicitly
    # These are non-instance variables for an object
    public segregate int DEFAULT_X = 0;
    public const segregate long DEFAULT_Y = 5; # Constant segregated variable

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

    # Singular constructor
    public constructor(int o)
    {
        this(o, o, o); # using 'this' as a constructor call
    }

    #####################################
    #       ACCESSORS IN OBJECTS        #
    #####################################

    # Mutator method
    public int setX(int nx)
    {
        # This public method can be accessed outside of the object's scope
        int ox = x;
        x = nx;
        return ox;
    }

    private int setY(int ny)
    {
        # This private method can only be accessed within the scope of this object
        int oy = y;
        x = ny;
        return oy;
    }

    protected int setZ(int nz)
    {
        # This is a protected method, meaning it can be accessed within the scope of this
        # object and the scope of subobjects ONLY
        int oz = z;
        x = nz;
        return oz;
    }

    public const Location zero() # Constant method; cannot be overridden in subobjects
    {
        x = 0;
        y = 0;
        z = 0;
        return this; # 'this' keyword is a reference to the object which it's being used in
    }

    public Location +=(int d) # Overloaded operator method for +=
    {
        x += d;
        y += d;
        z += d;
        return this;
    }

    # Segregate methods
    public segregate Location zeroed()
    {
        # Segregated methods can be used without creating an instance of the object
        return Location(0, 0, 0);
    }
}

# Usage

private void objUsage()
{
    Location standard = Location(5, 5, 5); # Object construction
    Location zero = Location.zeroed(); # Segregate method access using dot (.) operator
    standard.setX(10); # Valid, using public method in object
    # standard.setY(15); Error! Attempting to use private method in unauthorized scope
    # standard.setZ(30); Error again! Attempting to use protected method in unauthorized scope
    standard.zero(); # Zero all values in the location
    standard += 5; # Use operator overload to add 5 to x, y, and z
}

###############################################
#       CONCEPTUAL OBJECTS (ABSTRACTION)      #
###############################################

public object Animal
{
    public conceptual void speak(); # Conceptual method
    # This method declaration marks Animal as a conceptual object
    # Conceptual methods CANNOT be segregated as well
    # public conceptual segregate void play(); Invalid. Will result in a compiling error.
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

###############################
#       GENERIC OBJECTS       #
###############################

public object Container<T>
{
    private T value;

    public constructor(T v)
    {
        value = v;
    }

    public constructor() {}

    public E getValue()
    {
        return value;
    }
}

# Initialization

private void genExample()
{
    Container<int> container = Container<int>(5);
}

private Location locationHorizontal(int vx, int vz)
{
    return Location(vx, 100, vz); # Explicit, like types
}

private Location locationHorizontalExplicit(int vx, int vz)
{
    return Location(x = vx, y = 100, z = vz)
}

#########################
#       NAMESPACES      #
#########################

namespace Utilities # Utility namespace
{
    public int m2(int x)
    {
        return x + x;
    }
}

# Usage outside of namespace
private int nsUsage()
{
    # double colon (::) operator for scoping into namespaces
    return Utilities::m2(6); # Returns 12
}

# Nested namespaces

namespace Outer
{
    namespace Inner
    {
        public const int COUNT = 5; # Outer::Inner::COUNT to access
    }
}

#####################################
#       DEFINITION INCLUSION        #
#####################################

use printf; # defines 'printf' as a function which will be included at compile-time

public int defIncTest()
{
    printf("%d", 10); # use normally as a function
}

##############################
#       FILE INCLUSION       #
##############################

# asdf.as:
#
# public int test()
# {
#     return 10;
# }

use printf; # definition inclusion from c stdlib
use "asdf.as";

public int fileIncTest()
{
    printf("%d", test());
}

#################################
#       C LANG INCLUSIONS       #
#################################

# asdf.c:
#
# #include <stdio.h>
# 
# public void testTwo()
# {
#     printf("%d", 20);
# }

use native "asdf.c";

public int clangIncTest()
{
    testTwo(); # prints 20
}

######################################
#       MATHEMATICAL FUNCTIONS       #
######################################

# This function must produce a result and have no side effects
public mathematical int addition(int x, int y)
{
    return x + y;
}

#####################
#       ARRAYS      #
#####################

use int printf(char[], double);

public int arrayTest()
{
    # assign an integer to an array declaration to allocate x amount of elements
    int* later := 10; # this creates an array of 10 ints
    # assign an array literal to an array declaration
    double* now = [5.3, 6.4];
    # obtain and print a value
    printf("%f", now[0]); # result: 5.3
    # assign a new value to an array index
    now[1] = 7.3; # 6.4 has been overridden by 7.3 at index 1
    printf("%f", now[1]); # result: 7.3
}