#ifndef _MAIN_
#define _MAIN_

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "droplet_init.h"
#include "pc_com.h"
#include "motor.h"
#include "IRcom.h"

void pulse();
void check_messages();
void color_cycle();

#endif