#include <Windows.h>
struct charWithCoords
{   
    /*
    * a single "pixel" inside the shape 
    */
    char c;
    int x;
    int y;
    WORD atribute;
};