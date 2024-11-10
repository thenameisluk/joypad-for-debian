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
    virtualMouse()
    {
        dev = libevdev_new();
        libevdev_set_name(dev, "test device");
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
            throw std::string("unable to create device");
    }
    ~virtualMouse()
    {
        libevdev_uinput_destroy(uidev);
        libevdev_free(dev);
    }
    void move(int32_t x, int32_t y)
    {
        if (x)
            libevdev_uinput_write_event(uidev, EV_REL, REL_X, x);
        if (y)
            libevdev_uinput_write_event(uidev, EV_REL, REL_Y, y);
        sync();
    }
    //assignes buttons state
    //remember to run sync after in order to not introduce any issues
    void leftPress(int state)
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, state);
    }
    void midlePress(int state)
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_MIDDLE, state);
    }
    void rightPress(int state)
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_RIGHT, state);
    }
    //clicks button
    void leftClick()
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, 1);
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, 0);
        sync();
    }
    void midleClick()
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_MIDDLE, 1);
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_MIDDLE, 0);
        sync();
    }
    void rightClick()
    {
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_RIGHT, 1);
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_RIGHT, 0);
        sync();
    }
    void sync()
    {
        libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    }
};

class virtualKeyboard
{
public:
    int err;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    virtualKeyboard()
    {
        dev = libevdev_new();
        libevdev_set_name(dev, "test device");
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
            throw std::string("unable to create device");
    }
    //assignes buttons state
    //remember to run sync after in order to not introduce any issues
    void press(uint32_t key,int state)
    {
        libevdev_uinput_write_event(uidev, EV_KEY, key, state);
    }
    //clicks button
    void click(uint32_t key)
    {
        libevdev_uinput_write_event(uidev, EV_KEY, key, 1);
        libevdev_uinput_write_event(uidev, EV_KEY, key, 0);
        sync();
    }
    void sync()
    {
        libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    }
};