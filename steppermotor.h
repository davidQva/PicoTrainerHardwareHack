#pragma once


#include <stdbool.h>

extern volatile bool motor_fram;
extern volatile bool motor_bak;
extern volatile float resistance;

void step_motor(bool direction);