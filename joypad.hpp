#pragma once
#include <ioput.hpp>

void handleMouse();
void kandleKeyFields(input_event &ev);
void handleCont(input_event &ev);
void handleABS(input_event &ev);
void forwardGamepad(unsigned int type, unsigned int code, int value);