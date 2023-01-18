#include <Windows.h>
#include "user.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;


class Screen {
private:
    const int WIDTH;
    const int HEIGHT;

    HANDLE wHnd; /* write (output) handle */
    HANDLE rHnd; /* read (input handle */

    COORD characterBufferSize;
    COORD characterPosition;
    SMALL_RECT consoleWriteArea;

    CHAR_INFO* consoleBuffer;

    User p;

    int FDELAY = 1000 / 60;

public:
    Screen(int w, int h, User player) :WIDTH(w), HEIGHT(h) {

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

        SetConsoleTitle(L"Projek Kacper Pach");
    }

    void drawMatrix(std::vector<charWithCoords> obj, int x, int y) {
        // not accually drawing a "Matrix" it draws all objects inside a vector 
        for (charWithCoords o : obj) {
            writeToPos(x + o.x, y + o.y, o.c, o.atribute);
        }
    }

    void drawPlayer() {
        drawMatrix(p.shape, p.getX(), p.getY());
    }

    void readInputEvents(DWORD& numEvents, DWORD& numEventsRead, bool& appIsRunning, int eTime) {
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

                    // handling arrows
                    if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_RIGHT) {
                        p.moveWithBoundaries(0.02 * eTime, 0, WIDTH, HEIGHT);
                    }
                    if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_LEFT) {
                        p.moveWithBoundaries(-0.02 * eTime, 0, WIDTH, HEIGHT);
                    }
                    if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN) {
                        p.moveWithBoundaries(0, 0.02 * eTime, WIDTH, HEIGHT);
                    }
                    if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_UP) {
                        p.moveWithBoundaries(0, -0.02 * eTime, WIDTH, HEIGHT);
                    }
                    // handling + and -
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
        // technicly not only the loop but was too lazy to put those lines somewhere else
        DWORD numEvents = 0;
        DWORD numEventsRead = 0;
        bool appIsRunning = true;
        int elapsedTime = 100; // keeps time between frames
        float rainbowOffset = 0; 

        p.setPos(WIDTH / 2, HEIGHT / 2); // set initial position at the middle of the screen

        slowConsoleBufferClean(); 
        while (appIsRunning) {
            auto s = system_clock::now();
            readInputEvents(numEvents, numEvents, appIsRunning, elapsedTime);

            slowConsoleBufferClean();
            rainbowOffset += elapsedTime * 0.01;
            p.rainbowUpdate(rainbowOffset);
            drawPlayer();

            update();

            if (FDELAY > elapsedTime) // limit frames so the program doesn't double as a singe thread stress test 
                Sleep(FDELAY - elapsedTime);
            elapsedTime = duration_cast<milliseconds>(system_clock::now() - s).count();
        }
    }

    void writeToPos(int x, int y, char c, WORD atribute) {
        // draw character  at x, y 
        // hide the buffer logic
        consoleBuffer[x + WIDTH * y].Char.AsciiChar = c;
        consoleBuffer[x + WIDTH * y].Attributes = atribute;
    }

    void slowConsoleBufferClean() {
        // to lazy to keep track of what has been changed 
        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            consoleBuffer[i].Char.AsciiChar = ' ';
            consoleBuffer[i].Attributes = WORD(124);
        }
    }

    void update() {
        // redraw screen
        COORD characterBufferSize = { WIDTH, HEIGHT };
        COORD characterPosition = { 0, 0 };
        SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };
        WriteConsoleOutputA(wHnd, consoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);
    }

    ~ Screen() {
        delete[] consoleBuffer;
    }
};