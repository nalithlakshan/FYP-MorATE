#ifndef SETUP_AND_LOOP_H
#define SETUP_AND_LOOP_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <spi.h>
#include <i2c.h>
#include "structs.h"
#include "defines.h"

#include "mcp4728.h"
#include "pin_card.h"

int buzzerTime;
//
int DAC_CODE;


void main_loop();
void setup();
uint32_t VCP_read(uint8_t* Buf, uint32_t Len);
void print(char *msg,...);
void beep(int duration);
void led_kinghtrider();



#endif
