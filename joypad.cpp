#include <ioput.hpp>
#include <joypad.hpp>

virtualMouse mouse;
virtualKeyboard keyboard;
virtualGamepad gamepad;

int32_t lastx = 0;
int32_t lasty = 0;

uint8_t matrixMap[3][3] = {
    {1, 2, 3},
    {8, 0, 4},
    {7, 6, 5}};
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

void handleMouse(){
    mouse.move(lastx, lasty);
}
void kandleKeyFields(input_event &ev)
{
    uint32_t value = ev.value;

    if (ev.code == ABS_X)
    {
        mXIndex = 1;
        if (value < 300)
            mXIndex = 0;
        if (value > 700)
            mXIndex = 2;

        printf("X %d : %d\n", mXIndex, value);
    }

    if (ev.code == ABS_Y)
    {
        mYIndex = 1;
        if (value < 300)
            mYIndex = 0;
        if (value > 700)
            mYIndex = 2;
        printf("Y %d : %d\n", mYIndex, value);
    }
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

    printf("x %d y %d\n", mXIndex, mYIndex);
    int32_t index = matrixMap[mYIndex][mXIndex];

    if (ev.value == 1)
    {
        switch (ev.code)
        {
        case BTN_NORTH:
            keyboard.click(keyMatrix[index][0]);
            break;
        case BTN_EAST:
            keyboard.click(keyMatrix[index][1]);
            break;
        case BTN_SOUTH:
            keyboard.click(keyMatrix[index][2]);
            break;
        case BTN_WEST:
            keyboard.click(keyMatrix[index][3]);
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
    gamepad.write_event(type, code, value);
}