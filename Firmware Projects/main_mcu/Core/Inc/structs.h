#ifndef STRUCTS_H
#define STRUCTS_H

#include "defines.h"

typedef struct FIFO
{
	uint32_t head;
	uint32_t tail;
    uint8_t data[FIFO_SIZE];
} FIFO;

#endif
