#include <windows.h> /* for HANDLE type, and console functions */
#include <iostream>
#include "Screen.h"


int main()
{   

    // getting initial shape vaues from users 
    std::string inputchars;
    char finalchar; 
    int size(20);
    std::cout << "podaj char jaki chcesz uzyc: ";
    std::cin >> inputchars;
    finalchar = inputchars[0]; // < in case user types multiple chars take the first one

    while (size > 15) { // < limit size so the shape cannot be bigger then the screen
        std::cout << "podaj poczatkowy rozmiar figury: ";
        std::cin >> size;
    }

    User player;
    player.setShape(size, finalchar, WORD(124));

    // set rows & colums to size to however many fit into initial size of console 
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int rows = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int columns = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // init screen + set routine 
    Screen s(rows,columns,player);
    s.gameLoop();
}