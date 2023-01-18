#include <windows.h> /* for HANDLE type, and console functions */
#include <stdio.h> /* standard input/output */
#include <stdlib.h> /* included for rand */
#include <ctime>
#include <memory>
#include <string>
#include <chrono>
#include <vector>
#include <iostream>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

struct charWithCoords
{
    char c;
    int x;
    int y;
    WORD atribute;
};

class User {
private:
    float shapeSize;
    char defaultChar;
    WORD defaultColor;
    float posx = 0.0f, posy = 0.0f;

    bool isInsideBounds(int px, int py, int width, int height, int offset) {
        return (px >= -1*offset -1 + shapeSize && px <= +1 * offset + width - shapeSize && py >= -1 * offset -1 + shapeSize && py <= +1 * offset + height - shapeSize);
    }

public:
    
    std::vector<charWithCoords> shape;

    void setShape(float size, char c, WORD color) {
        shapeSize = size;
        defaultChar = c;
        defaultColor = color;
        shape.clear();

        shape.push_back({ c,0,0,color });

        for (int i = 1; i < size; i++)
        {
            shape.push_back({ c,  1 * i, -1 * i, color });
            shape.push_back({ c, -1 * i, -1 * i, color });
            shape.push_back({ c, -1 * i,  1 * i, color });
        }
    }

    void changeSize(float diff, int width, int height) {
        if (isInsideBounds(posx, posy, width, height, -3) || diff < 0) {
            setShape(shapeSize + diff, defaultChar, defaultColor);
            //oszukanie punktu zaczepienia w środku 
            int centerx = width / 2;
            int centery = height / 2;
            if (posx - centerx < 0) {
                move(-1 * diff, 0);
            }
            else if (posx - centerx > 0) {
                move(1 * diff, 0);
            }
            if (posy - centery < 0) {
                move(0, -1 * diff);
            }
            else if (posy - centery > 0) {
                move(0, 1 * diff);
            }
        }
    }

    void rainbowUpdate(int offset) {
        //0 -> 7 7 -> 0 
        const float w = 0.14;

        shape[0].atribute = 120 + sin((offset-w) * w) * 4 + 4;

        for (int i = 0; i < shapeSize-1; i++)
        {
            shape[i * 3 + 1].atribute = 120 + sin((i + offset) * w) * 4 + 4;
            shape[i * 3 + 2].atribute = 120 + sin((i + offset) * w) * 4 + 4;
            shape[i * 3 + 3].atribute = 120 + sin((i + offset) * w) * 4 + 4;
        }

    }

    void setPos(float x, float y) {
        posx = x;
        posy = y;
    }

    void moveWithBoundaries(float x, float y, int width, int height) {
        int newPlayerX = (int)(posx + x);
        int newPlayery = (int)(posy + y);
        //if (xbound >= -1 + shapeSize && xbound <= width - shapeSize && ybound >= -1 + shapeSize && ybound <= height - shapeSize)
        if(isInsideBounds(newPlayerX, newPlayery,width,height,-1))
        {
            move(x, y);
        }
    }

    void move(float x, float y) {
        setPos(posx + x, posy + y);
    }

    int getX() {
        return (int)posx;
    }

    int getY() {
        return (int)posy;
    }
};

class Screen {
    private:
        bool isFrog;

        const int WIDTH;
        const int HEIGHT;

        HANDLE wHnd; /* write (output) handle */
        HANDLE rHnd; /* read (input handle */

        COORD characterBufferSize;
        COORD characterPosition;
        SMALL_RECT consoleWriteArea;

        CHAR_INFO *consoleBuffer;

        User p;

        int FDELAY = 1000 / 60;

    public:
        Screen(int w, int h, User player) :WIDTH(w),HEIGHT(h) {

            p = player;

            /* Window size coordinates, be sure to start index at zero! */
            SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };

            /* A COORD struct for specificying the console's screen buffer dimensions */
            COORD bufferSize = { WIDTH, HEIGHT };

            /* Setting up different variables for passing to WriteConsoleOutput */
            characterBufferSize = { (short)WIDTH, (short)HEIGHT };
            characterPosition = { 0, 0 };
            consoleWriteArea = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };

            /* A CHAR_INFO structure containing data about a single character */
            consoleBuffer = new CHAR_INFO[WIDTH * HEIGHT];

            /* initialize handles */
            wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
            rHnd = GetStdHandle(STD_INPUT_HANDLE);

            /* Set the window size */
            SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

            /* Set the screen's buffer size */
            SetConsoleScreenBufferSize(wHnd, bufferSize);
        }

        void drawMatrix(std::vector<charWithCoords> obj, int x, int y) {
            for (charWithCoords o : obj) {
                writeToPos(x + o.x, y + o.y, o.c, o.atribute);
            }
        }
        
        void drawPlayer() {
            drawMatrix(p.shape, p.getX(), p.getY());
        }

        void readInputEvents(DWORD &numEvents, DWORD &numEventsRead, bool &appIsRunning, int eTime) {
            // Find out how many console events have happened:
            GetNumberOfConsoleInputEvents(rHnd, &numEvents);

            // If it's not zero (something happened...)
            if (numEvents != 0) {

                // Create a buffer of that size to store the events
                INPUT_RECORD* eventBuffer = new INPUT_RECORD[numEvents];

                // Read the console events into that buffer, and save how
                // many events have been read into numEventsRead.
                ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);

                // Now, cycle through all the events that have happened:
                for (DWORD i = 0; i < numEventsRead; ++i) {

                    // Check the event type: was it a key?
                    if (eventBuffer[i].EventType == KEY_EVENT) {

                        // Yes! Was the key code the escape key?
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {

                            // Yes, it was, so set the appIsRunning to false.
                            appIsRunning = false;
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_RIGHT) {
                            p.moveWithBoundaries(0.02 * eTime, 0, WIDTH, HEIGHT);
                            //p.move(0.02 * eTime, 0);
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_LEFT) {
                            p.moveWithBoundaries(- 0.02 * eTime, 0, WIDTH, HEIGHT);
                            //p.move( - 0.02 * eTime,0);
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN) {
                            p.moveWithBoundaries(0, 0.02 * eTime, WIDTH, HEIGHT);
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_UP) {
                            p.moveWithBoundaries(0, -0.02 * eTime, WIDTH, HEIGHT);
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_OEM_PLUS) {
                            p.changeSize(0.015 * eTime, WIDTH, HEIGHT);
                        }
                        if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_OEM_MINUS) {
                            p.changeSize(-0.015 * eTime, WIDTH, HEIGHT);
                        }
                    }
                }

                // Clean up our event buffer:
                delete[] eventBuffer;
            }
        }

        void gameLoop() {
            DWORD numEvents = 0;
            DWORD numEventsRead = 0;
            bool appIsRunning = true;
            int elapsedTime = 100;
            float r = 0;

            p.setPos(WIDTH / 2, HEIGHT / 2);

            slowConsoleBufferClean();
            while (appIsRunning) {
                auto s =  system_clock::now();
                readInputEvents(numEvents, numEvents, appIsRunning, elapsedTime);

                slowConsoleBufferClean();
                r+= elapsedTime*0.01;
                p.rainbowUpdate(r);
                drawPlayer();

                update();

                wchar_t buffer[256];
                wsprintfW(buffer, L"%d", elapsedTime);

                SetConsoleTitle(buffer);

                if (FDELAY > elapsedTime)
                    Sleep(FDELAY - elapsedTime);
                elapsedTime = duration_cast<milliseconds>(system_clock::now() - s).count();
                
            }
        }

        void writeToPos(int x, int y, char c, WORD atribute) {

            consoleBuffer[x + WIDTH * y].Char.AsciiChar = c;
            consoleBuffer[x + WIDTH * y].Attributes = atribute;
        }

        void slowConsoleBufferClean() {
            for (int i = 0; i < WIDTH*HEIGHT; i++)
            {
                consoleBuffer[i].Char.AsciiChar = ' ';
                consoleBuffer[i].Attributes = WORD(124);
            }
        }

        void update() {
            COORD characterBufferSize = { WIDTH, HEIGHT };
            COORD characterPosition = { 0, 0 };
            SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };
            WriteConsoleOutputA(wHnd, consoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);
        }

        void cleanMem() {
            delete[] consoleBuffer;
        }
};

int main(void)
{   
    std::cout << "podaj char jaki chcesz uzyc: ";
    char c;
    std::cin >> c;
    std::cout << "podaj poczatkowy rozmiar figury: ";
    int size;
    std::cin >> size;

    User player;
    player.setShape(size, c, WORD(124));

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int rows = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int columns = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    Screen s(rows,columns,player);

    s.gameLoop();
    s.cleanMem();
}