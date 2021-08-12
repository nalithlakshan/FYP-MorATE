#ifndef PIN_CARD_H
#define PIN_CARD_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <spi.h>
#include <i2c.h>
#include "structs.h"
#include "defines.h"

#include "setup_and_loop.h"
#include "fpga.h"
extern uint8_t transmit_buffer[TX_BUFFER_SIZE];
extern FIFO RX_FIFO ;
extern int totRX;


int read_id;
int packet_size;
uint16_t payload_size;
int device;
int command;
int value;

int count;
uint8_t packet[4096];


/// flags
uint8_t new_packet_read ; //a new packet is read
uint8_t packet_sent; // a new packet is sent
uint8_t read_from_ui;
uint8_t request_from_pincard; // when this is high, requesting should be started
uint8_t notify_new_ic;	//
uint8_t digital_tests;

uint8_t header_to_ui[10];


int num_req_A ;
int num_req_B ;
int num_packets_to_A;
int num_packets_from_ui;


unsigned long long lt_packet;
unsigned long long ct_packet;

uint8_t request_status;

void requst_from_pin_card();
uint8_t decode_packet();
uint8_t read_packet_FIFO();
void reset_values();

#endif

