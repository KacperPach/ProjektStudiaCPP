#include <Windows.h>
#include <vector>
#include <chrono>
#include "misc.h"

class User {

private:
    float shapeSize;
    char defaultChar;
    WORD defaultColor;
    float posx = 0.0f, posy = 0.0f; // position keept in floats because of real time positioning 

    bool isInsideBounds(int px, int py, int width, int height, int offset) {
        return (px >= -1 * offset - 1 + shapeSize && px <= +1 * offset + width - shapeSize && py >= -1 * offset - 1 + shapeSize && py <= +1 * offset + height - shapeSize);
    }

public:
    
    std::vector<charWithCoords> shape; // strusture made to keep track of every element inside shape 

    void setShape(float size, char c, WORD color) {
        shapeSize = size;
        defaultChar = c;
        defaultColor = color;
        shape.clear();

        shape.push_back({ c,0,0,color });

        for (int i = 1; i < size; i++) // calculates shape 
        {
            shape.push_back({ c,  1 * i, -1 * i, color });
            shape.push_back({ c, -1 * i, -1 * i, color });
            shape.push_back({ c, -1 * i,  1 * i, color });
        }
    }

    void changeSize(float diff, int width, int height) {
        // non optimised (slow) shape size change with bounds check
        if (isInsideBounds(posx, posy, width, height, -3) || diff < 0) {
            setShape(shapeSize + diff, defaultChar, defaultColor);
            
            // siulating ancer point in the middle? not shure if I iterpreted this point correctly 
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
        const float w = 0.14; // controls width of a single color 

        shape[0].atribute = 120 + sin((offset - w) * w) * 4 + 4; // sighn to make it look natural tbh equasion created
                                                                 // by "trial and error" until it looked good. 

        for (int i = 0; i < shapeSize - 1; i++)
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
        if (isInsideBounds(newPlayerX, newPlayery, width, height, -1))
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
