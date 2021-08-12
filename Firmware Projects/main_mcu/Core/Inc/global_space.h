#ifndef GLOBAL_SPACE_H
#define GLOBAL_SPACE_H

#include "structs.h"

uint8_t transmit_buffer[TX_BUFFER_SIZE] = "";
uint8_t receive_buffer[RX_BUFFER_SIZE] = "";
FIFO RX_FIFO = {.head=0, .tail=0};

int totRX = 0;



#endif
