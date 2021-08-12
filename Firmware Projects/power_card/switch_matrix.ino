#include "defines.h"

// void shiftOut8(BITORDER bitOrder, uint8_t val);
// void shiftOutAll(void);
// void matrix_init(void);

// ################# Function Declaration ###################

void shiftOut8(BITORDER bitOrder, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {

		if (bitOrder == LSBF) {
			digitalWrite(SHIFT_DATA_PIN, val & 1);
			val = val>>1;
		} else {
			digitalWrite(SHIFT_DATA_PIN, (val & 128) != 0);
			//Serial1.println((val & 128) != 0);
			//Serial1.println(val);
			val = val<<1;
			//Serial1.println(val);
			//Serial1.println();
			
		}
		for(uint8_t j = 0; j < 4; j++){} // last delay was 4

		digitalWrite(SHIFT_CLK, 1);
		digitalWrite(SHIFT_CLK, 0);
	}
}

void shiftOutAll(void)
{
	uint8_t i;
	for(i = 0; i < 21; i++)
	{
		shiftOut(SHIFT_DATA_PIN, SHIFT_CLK,MSBFIRST,matrix_array[i]);
	}
	digitalWrite(STCP, 0);
	digitalWrite(STCP, 1);
	digitalWrite(STCP, 0);

	delay(20);
}

void matrix_init(void)
{
	digitalWrite(LED_D5,0);
	digitalWrite(OE,1);
	delay(50);
	for(uint8_t i = 0; i <21; i++)
	{
		matrix_array[i] = 0; 
	}
	
	shiftOutAll();
	digitalWrite(RST, 0);
	delay(100);
	digitalWrite(RST, 1);
	delay(100);
	digitalWrite(OE, 0);
	digitalWrite(STCP, 0);
	digitalWrite(STCP, 1);
	digitalWrite(STCP, 0);

	digitalWrite(LED_D5,1);
	delay(500);
}
