
int main()
{
    // A compendium of unsafe operations

    // Values
    int x;      // Error - uninitialized

    // Pointers
    int *p;     // Error - uninitialized
    *p = 42;    // Error - uninitialized assignment
    int y = *p; // Error - uninitialized dereference

    int *q = nullptr;
    *q = 42;    // Error - null pointer assignment
    int z = *q; // Error - null pointer dereference

    // Error - dangling pointer
    auto fn = []() { int x; return &x; };

    // Use after free
    int *r = new int;
    delete r;
    *r = 42;    // Error - dangling pointer - assignment after free
    int w = *r; // Error - dangling pointer - read after free

    // References
    int *a = new int;
    int &b = *a;
    delete a;
    b = 12;     // Error - dangling write - assignment after free
    int c = b;  // Error - dangling read - read after free

    // Error - dangling reference
    auto fn2 = []() -> int & { int x; return x; };

    // Arrays
    int array[3];
    array[3] = 42;  // Error - out of bounds
    array[-1] = 42; // Error - out of bounds
    c = array[-1];  // Error - out of bounds
    c = array[3];   // Error - out of bounds

    // Functions that are unsafe
    //memcpy
    //sprintf
    // Out of bounds copy



    // Casts


    // Strings

    // Vectors



}
