/*
 * si5351.h
 *
 *  Created on: Jan 17, 2021
 *      Author: Thilina
 */

#ifndef INC_SI5351_H_
#define INC_SI5351_H_


#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <spi.h>
#include <i2c.h>
#include "structs.h"
#include "defines.h"

// default I2C address of the Si5351
#define SIADDR 0x60

// register's power modifiers
#define SIOUT_2mA 0
#define SIOUT_4mA 1
#define SIOUT_6mA 2
#define SIOUT_8mA 3

// registers base (2mA by default)
#define SICLK0_R   76       // 0b1001100
#define SICLK12_R 108       // 0b1101100
#define int_xtal 27000000L  

uint16_t omsynth[3];
uint8_t clkOn[3];
uint8_t clkpower[3];

void clk_init();
void clk_reset();
void clk_set_frequency(uint8_t clk, uint32_t freq);
void clk_enable(uint8_t clk);
void clk_disable(uint8_t clk);
void clk_set_power(uint8_t clk, uint8_t power);
void clk_i2c_write(uint8_t regist, uint8_t value);
void clk_off();


#endif /* INC_SI5351_H_ */
