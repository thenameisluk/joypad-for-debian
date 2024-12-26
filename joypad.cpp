#include <ioput.hpp>
#include <joypad.hpp>
#include <iostream>
#include <cmath>

virtualMouse mouse = virtualMouse();
virtualKeyboard keyboard = virtualKeyboard();
virtualGamepad gamepad = virtualGamepad();

// setting env DEBUG to any value will trigger debbuging mode
bool debuging = (bool)std::getenv("DEBUG");

int32_t lastx = 0;
int32_t lasty = 0;
bool L2 = false;
bool R2 = false;

int32_t keyMatrix[9][4] = {
    /*
    x a b y
     x
    y a
     b

    123
    804
    765
    */
    {KEY_CAPSLOCK, KEY_SPACE, KEY_ENTER, KEY_BACKSPACE}, // 0
    {KEY_Q, KEY_W, KEY_S, KEY_A},                        // 1
    {KEY_E, KEY_R, KEY_F, KEY_D},                        // 2
    {KEY_T, KEY_Y, KEY_H, KEY_G},                        // 3
    {KEY_U, KEY_I, KEY_K, KEY_J},                        // 4
    {KEY_O, KEY_P, KEY_B, KEY_L},                        // 5
    {KEY_Z, KEY_X, KEY_C, KEY_V},                        // 6
    {KEY_N, KEY_M, KEY_SEMICOLON, KEY_BACKSLASH},        // 7
    {KEY_DOT, KEY_COMMA, KEY_QUESTION, KEY_LEFTMETA},    // 8
};

uint8_t mXIndex = 0;
uint8_t mYIndex = 0;

int scrollTimer = 0;

void handleMouse()
{
    if (lastx || lasty)
        mouse.move(lastx, lasty);

    if ((R2 ^ L2)&!scrollTimer)
    {
        if (L2)
        {
            mouse.scroll(1);
        }
        else
        {
            mouse.scroll(-1);
        }
    }
    scrollTimer++;
    scrollTimer%=16;
}

int32_t x = 500;
int32_t y = 500;
int32_t keyIndex = 0;
float rad = 0;
float degree = 0;
float dist = 0;

// make sure the ABS values are withing range of 0 to 1024
// and virtual-gamepad with this values looks correct in any
// gamepad testing program (x or y 's not inverted)
void kandleKeyFields(input_event &ev)
{
    uint32_t value = ev.value;

    if (ev.code == ABS_X)
    {
        x = value;
    }
    else if (ev.code == ABS_Y)
    {
        y = value;
    }
    else
        return;

    // calculating degree
    float posx = -512 + x;
    float posy = 512 - y;

    if (x == 0 && y == 0)
        return;

    dist = sqrt((posx * posx) + (posy * posy));

    rad = asin(posy / dist);

    if (posx < 0)
    {
        rad = M_PI - rad;
    }
    else if (posy < 0)
    {
        rad = (2 * M_PI) + rad;
    }

    degree = rad * 180.0f / M_PI;

    if (debuging)
        std::cout << " Degree: " << degree << " Dist: " << dist << std::endl;

    // getting field
    if (dist < 100)
    {
        keyIndex = 0;
    }
    else if (degree <= 22.5f)
    {
        keyIndex = 4;
    }
    else if (degree <= 22.5f + (45.0f * 1.0f))
    {
        keyIndex = 3;
    }
    else if (degree <= 22.5f + (45.0f * 2.0f))
    {
        keyIndex = 2;
    }
    else if (degree <= 22.5f + (45.0f * 3.0f))
    {
        keyIndex = 1;
    }
    else if (degree <= 22.5f + (45.0f * 4.0f))
    {
        keyIndex = 8;
    }
    else if (degree <= 22.5f + (45.0f * 5.0f))
    {
        keyIndex = 7;
    }
    else if (degree <= 22.5f + (45.0f * 6.0f))
    {
        keyIndex = 6;
    }
    else if (degree <= 22.5f + (45.0f * 7.0f))
    {
        keyIndex = 5;
    }
    else
    {
        keyIndex = 4;
    }

    if (debuging)
        std::cout << "Index: " << keyIndex << std::endl;
}

void handleCont(input_event &ev)
{
    if (ev.code == BTN_TL)
    {
        mouse.leftPress(ev.value);
        mouse.sync(); // all night debugging for this :[

        printf("left %d\n", ev.value);
    }

    if (ev.code == BTN_TR)
    {
        mouse.rightPress(ev.value);
        mouse.sync(); // all night debugging for this :[
        printf("right %d\n", ev.value);
    }

    if (ev.code == BTN_TL2)
        L2 = ev.value;

    if (ev.code == BTN_TR2)
        R2 = ev.value;

    if (ev.value == 1)
    {
        switch (ev.code)
        {
        case BTN_NORTH:
            keyboard.click(keyMatrix[keyIndex][0]);
            break;
        case BTN_EAST:
            keyboard.click(keyMatrix[keyIndex][1]);
            break;
        case BTN_SOUTH:
            keyboard.click(keyMatrix[keyIndex][2]);
            break;
        case BTN_WEST:
            keyboard.click(keyMatrix[keyIndex][3]);
            break;
        case BTN_DPAD_UP:
            keyboard.click(KEY_UP);
            break;
        case BTN_DPAD_LEFT:
            keyboard.click(KEY_LEFT);
            break;
        case BTN_DPAD_RIGHT:
            keyboard.click(KEY_RIGHT);
            break;
        case BTN_DPAD_DOWN:
            keyboard.click(KEY_DOWN);
            break;
        }
    }

    switch (ev.code)
    {
    case BTN_START:
        keyboard.press(KEY_LEFTCTRL, ev.value);
        break;
    case BTN_SELECT:
        keyboard.press(KEY_LEFTALT, ev.value);
        break;
    }
}

void handleABS(input_event &ev)
{
    if (ev.code == ABS_X || ev.code == ABS_Y)
        kandleKeyFields(ev);

    if (ev.code == ABS_RX)
    {
        lastx = 0;

        if (ev.value >= 600)
            lastx = 1;
        if (ev.value >= 900)
            lastx = 4;

        if (ev.value <= 400)
            lastx = -1;
        if (ev.value <= 100)
            lastx = -4;
    }
    if (ev.code == ABS_RY)
    {
        lasty = 0;

        if (ev.value >= 600)
            lasty = 1;
        if (ev.value >= 900)
            lasty = 4;

        if (ev.value <= 400)
            lasty = -1;
        if (ev.value <= 100)
            lasty = -4;
    }
}

void forwardGamepad(unsigned int type, unsigned int code, int value)
{
    if (debuging)
    {
        input_event ev;
        ev.type = type;
        ev.code = code;
        ev.value = value;

        inputEvent(ev).print();
    }
    gamepad.write_event(type, code, value);
}