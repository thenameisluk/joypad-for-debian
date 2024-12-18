#pragma once
/*
Usefull wrapper around libevdev for C++
Feel free to use in your own project

The MIT License

Copyright 2024 luk<luk@iaml.uk>

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <string>
#include <functional>
#include <stdexcept>
#include <thread>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

class inputEvent
{
public:
    uint16_t type;
    uint16_t code;
    int32_t value;
    inputEvent(input_event ev)
    {
        type = ev.type;
        code = ev.code;
        value = ev.value;
    }
    std::string getTypeName()
    {
        return libevdev_event_type_get_name(type);
    }
    std::string getCodeName()
    {
        return libevdev_event_code_get_name(type, code);
    }
    void print()
    {
        printf("Event: %s %s %d\n",
               libevdev_event_type_get_name(type),
               libevdev_event_code_get_name(type, code),
               value);
    }
};

class inputDevice
{
public:
    struct libevdev *dev = NULL;
    int fd;
    int rc = 1;
    inputDevice(const char *path, bool blocking = false)
    {
        fd = open(path, O_RDONLY | O_NONBLOCK);
        rc = libevdev_new_from_fd(fd, &dev);

        if (rc < 0)
        {
            throw std::string("failed to open ").append(path);
        }
    }
    std::string getName()
    {
        return libevdev_get_name(dev);
    }

    int getBusType()
    {
        return libevdev_get_id_bustype(dev);
    }
    int getVendor()
    {
        return libevdev_get_id_vendor(dev);
    }
    int getProduct()
    {
        return libevdev_get_id_product(dev);
    }

    void print()
    {
        printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
        printf("Input device ID: bus %#x vendor %#x product %#x\n",
               libevdev_get_id_bustype(dev),
               libevdev_get_id_vendor(dev),
               libevdev_get_id_product(dev));
    }
    // 0 - there is an event
    // other ignore ig
    bool manPull(input_event &ev)
    {
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 1 || rc == 0 || rc == -EAGAIN)
            return rc;
        throw "error";
    }
};
// fix it or sth
class virtualMouse
{
public:
    int err;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    virtualMouse(const char* name = "virtual mouse")
    {
        dev = libevdev_new();
        libevdev_set_id_vendor(dev,0);
        libevdev_set_id_product(dev,0);
        libevdev_set_name(dev, name);

        libevdev_enable_event_type(dev, EV_SYN);
        libevdev_enable_event_code(dev, EV_SYN, SYN_REPORT, NULL);
        libevdev_enable_event_type(dev, EV_REL);
        libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
        libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
        libevdev_enable_event_type(dev, EV_KEY);
        libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);

        err = libevdev_uinput_create_from_device(dev,
                                                 LIBEVDEV_UINPUT_OPEN_MANAGED,
                                                 &uidev);
        if (err != 0)
            throw std::runtime_error("unable to create device, are you root?");
    }
    ~virtualMouse()
    {
        libevdev_uinput_destroy(uidev);
        libevdev_free(dev);
    }
    void move(int32_t x, int32_t y)
    {
        if (x)
            write_event(EV_REL, REL_X, x);
        if (y)
            write_event(EV_REL, REL_Y, y);
        sync();
    }
    //assignes buttons state
    //remember to run sync after in order to not introduce any issues
    void leftPress(int state)
    {
        write_event(EV_KEY, BTN_LEFT, state);
    }
    void midlePress(int state)
    {
        write_event(EV_KEY, BTN_MIDDLE, state);
    }
    void rightPress(int state)
    {
        write_event(EV_KEY, BTN_RIGHT, state);
    }
    //clicks button
    void leftClick()
    {
        write_event(EV_KEY, BTN_LEFT, 1);
        write_event(EV_KEY, BTN_LEFT, 0);
        sync();
    }
    void midleClick()
    {
        write_event(EV_KEY, BTN_MIDDLE, 1);
        write_event(EV_KEY, BTN_MIDDLE, 0);
        sync();
    }
    void rightClick()
    {
        write_event(EV_KEY, BTN_RIGHT, 1);
        write_event(EV_KEY, BTN_RIGHT, 0);
        sync();
    }
    void write_event(unsigned int type, unsigned int code, int value){
        libevdev_uinput_write_event(uidev,type,code,value);
    }
    void sync()
    {
        write_event(EV_SYN, SYN_REPORT, 0);
    }
};

class virtualKeyboard
{
public:
    int err;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    virtualKeyboard(const char* name = "virtual keyboard")
    {
        dev = libevdev_new();
        libevdev_set_id_vendor(dev,0);
        libevdev_set_id_product(dev,0);
        libevdev_set_name(dev, name);

        libevdev_enable_event_type(dev, EV_SYN);
        libevdev_enable_event_code(dev, EV_SYN, SYN_REPORT, NULL);

        for (int i = 1; i <= 255; i++)
        { // register keyboard keys?
            libevdev_enable_event_code(dev, EV_KEY, i, NULL);
        }

        err = libevdev_uinput_create_from_device(dev,
                                                 LIBEVDEV_UINPUT_OPEN_MANAGED,
                                                 &uidev);
        if (err != 0)
            throw std::runtime_error("unable to create device, are you root?");
    }
    //assignes buttons state
    //remember to run sync after in order to not introduce any issues
    void press(uint32_t key,int state)
    {
        write_event(EV_KEY, key, state);
    }
    //clicks button
    void click(uint32_t key)
    {
        write_event(EV_KEY, key, 1);
        write_event(EV_KEY, key, 0);
        sync();
    }
    void write_event(unsigned int type, unsigned int code, int value){
        libevdev_uinput_write_event(uidev,type,code,value);
    }
    void sync()
    {
        write_event(EV_SYN, SYN_REPORT, 0);
    }
};

class virtualGamepad
{
public:
    int err;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    virtualGamepad(const char* name = "virtual gamepad",int32_t joymin = 0,int32_t joymax = 1000)
    {
        dev = libevdev_new();
        libevdev_set_id_vendor(dev,0);
        libevdev_set_id_product(dev,0);
        libevdev_set_name(dev, name);

        libevdev_enable_event_type(dev, EV_SYN);
        libevdev_enable_event_code(dev, EV_SYN, SYN_REPORT, NULL);
        //buttons
        libevdev_enable_event_type(dev, EV_KEY);
        libevdev_enable_event_code(dev, EV_KEY, BTN_GAMEPAD, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_NORTH, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_EAST, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_SOUTH, NULL);//same as gamepad btn but whatever
        libevdev_enable_event_code(dev, EV_KEY, BTN_WEST, NULL);
        //d-pad
        libevdev_enable_event_code(dev, EV_KEY, BTN_DPAD_UP, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_DPAD_LEFT, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_DPAD_DOWN, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_DPAD_RIGHT, NULL);
        //joysticks
        input_absinfo abs_info;
        abs_info.flat = 10;
        abs_info.fuzz = 0;
        abs_info.value = 0;
        abs_info.minimum = joymin;//default 0
        abs_info.maximum = joymax;//default 1000
        abs_info.resolution = 0;
        libevdev_enable_event_code(dev, EV_ABS, ABS_X, &abs_info);
        libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &abs_info);
        libevdev_enable_event_code(dev, EV_ABS, ABS_RX, &abs_info);
        libevdev_enable_event_code(dev, EV_ABS, ABS_RY, &abs_info);
        //joystick btn
        libevdev_enable_event_code(dev, EV_KEY, BTN_THUMBL, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_THUMB2, NULL);
        //trigger
        libevdev_enable_event_code(dev, EV_KEY, BTN_TR, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_TR2, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_TL, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_TL2, NULL);
        //menu
        libevdev_enable_event_code(dev, EV_KEY, BTN_START, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_SELECT, NULL);
        libevdev_enable_event_code(dev, EV_KEY, BTN_MODE, NULL);

        err = libevdev_uinput_create_from_device(dev,
                                                 LIBEVDEV_UINPUT_OPEN_MANAGED,
                                                 &uidev);
        if (err != 0)
            throw std::runtime_error("unable to create device, are you root?");
    }
    //assignes buttons state
    //remember to run sync after in order to not introduce any issues
    void press(uint32_t key,int state)
    {
        write_event(EV_KEY, key, state);
    }
    //clicks button
    void click(uint32_t key)
    {
        write_event(EV_KEY, key, 1);
        write_event(EV_KEY, key, 0);
        sync();
    }
    void write_event(unsigned int type, unsigned int code, int value){
        libevdev_uinput_write_event(uidev,type,code,value);
    }
    void sync()
    {
        write_event(EV_SYN, SYN_REPORT, 0);
    }
};