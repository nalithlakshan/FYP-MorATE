/*
 * fpga.h
 *
 *  Created on: Jan 16, 2021
 *      Author: Thilina
 */

#ifndef INC_FPGA_H_
#define INC_FPGA_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <spi.h>
#include <i2c.h>
#include <stdbool.h>
#include "structs.h"
#include "defines.h"
#include "mcp4728.h"

#define NO_REQ 0
#define SPI_WRREQ 1
#define SPI_RDREQ 2
#define COPY_FROM_SPI 3
#define COPY_TO_SPI 4
#define EXE_START 5
#define CLOCK_BYPASS 6

//test receive vectors
uint8_t IO4_capture[2];
uint8_t IO7_capture[2];

uint8_t null_byte[1];

uint8_t fdata_to_ui[4096];

//IO shift registers
// uint8_t io_config_matrix[12];
uint8_t sr[12];

typedef enum bitorder
{
  LSBF = 0,
  MSBF = 1,
}BITORDER;

typedef enum io_direction
{
	IO_INPUT,
	IO_OUTPUT,
}IO_DIRECTION;

void f_reset();
void f_request(int req);
void f_address(int address);
void f_testbench();
void f_load_source();
void f_set_frequency(int freq);
void f_set_io_voltage(int code);
// void shiftOut8(BITORDER bitOrder, uint8_t val);
void f_shiftOutAll(void);
void f_matrix_init(void);
void f_set_io(int io, int pinmode);




#endif /* INC_FPGA_H_ */
