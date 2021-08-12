/*
 * fpga.c
 *
 *  Created on: Jan 16, 2021
 *      Author: Thilina
 */
#include "fpga.h"

void f_reset()
{
	HAL_GPIO_WritePin(F_ADDR0_GPIO_Port, F_ADDR0_Pin, 0);
	HAL_GPIO_WritePin(F_ADDR1_GPIO_Port, F_ADDR1_Pin, 0);
	HAL_GPIO_WritePin(F_ADDR2_GPIO_Port, F_ADDR2_Pin, 0);
	HAL_GPIO_WritePin(F_ADDR3_GPIO_Port, F_ADDR3_Pin, 0);

	HAL_GPIO_WritePin(F_CS0_GPIO_Port, F_CS0_Pin, 0);
	HAL_GPIO_WritePin(F_CS1_GPIO_Port, F_CS1_Pin, 0);

	HAL_GPIO_WritePin(F_RST_GPIO_Port, F_RST_Pin, 1);
	HAL_Delay(2);
	HAL_GPIO_WritePin(F_RST_GPIO_Port, F_RST_Pin, 0);
	HAL_Delay(2);
	HAL_GPIO_WritePin(F_RST_GPIO_Port, F_RST_Pin, 1);

	f_matrix_init();

}

void f_request(int req)
{
	int bit0 = (req & 0b1)>0? 1:0;
	int bit1 = (req & 0b10)>0? 1:0;
	int bit2 = (req & 0b100)>0? 1:0;

	HAL_GPIO_WritePin(F_REQ0_GPIO_Port,F_REQ0_Pin,bit0);
	HAL_GPIO_WritePin(F_REQ1_GPIO_Port,F_REQ1_Pin,bit1);
	HAL_GPIO_WritePin(F_REQ2_GPIO_Port,F_REQ2_Pin,bit2);

	//process request after every request
	HAL_GPIO_WritePin(F_PROCESS_REQ_GPIO_Port, F_PROCESS_REQ_Pin, 1);
	HAL_Delay(5);
	HAL_GPIO_WritePin(F_PROCESS_REQ_GPIO_Port, F_PROCESS_REQ_Pin, 0);
}

void f_address(int address)
{
	int bit0 = (address & 0b1)>0? 1:0;
	int bit1 = (address & 0b10)>0? 1:0;
	int bit2 = (address & 0b100)>0? 1:0;
	int bit3 = (address & 0b1000)>0? 1:0;

	HAL_GPIO_WritePin(F_ADDR0_GPIO_Port, F_ADDR0_Pin, bit0);
	HAL_GPIO_WritePin(F_ADDR1_GPIO_Port, F_ADDR1_Pin, bit1);
	HAL_GPIO_WritePin(F_ADDR2_GPIO_Port, F_ADDR2_Pin, bit2);
	HAL_GPIO_WritePin(F_ADDR3_GPIO_Port, F_ADDR3_Pin, bit3);

}


void f_cs(int device)
{
	int bit0 = (device & 0b1)>0? 1:0;
	int bit1 = (device & 0b10)>0? 1:0;

	HAL_GPIO_WritePin(F_CS0_GPIO_Port,F_CS0_Pin, bit0);
	HAL_GPIO_WritePin(F_CS1_GPIO_Port,F_CS1_Pin, bit1);
	HAL_Delay(2);

}

void f_testbench()
{
	uint8_t IO2_source[1] = {0b11001100};
	uint8_t IO3_source[1] = {0b10101010};
	uint8_t IO5_source[1] = {0b00110011};
	uint8_t IO6_source[1] = {0b01010101};
	uint8_t null_byte[1] = {0b00000000};



	f_cs(1);

	//set data to IO2 begin
	f_request(SPI_WRREQ);
	HAL_SPI_Transmit(&hspi1, IO2_source, 1, 10);
	f_request(NO_REQ);
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);

	f_address(1);
	f_request(COPY_FROM_SPI);
	f_request(NO_REQ);

	//HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//set data to IO2 end

	//set data to IO3 begin
	f_request(SPI_WRREQ);
	HAL_SPI_Transmit(&hspi1, IO3_source, 1, 10);
	f_request(NO_REQ);
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);

	f_address(2);
	f_request(COPY_FROM_SPI);
	f_request(NO_REQ);

	//HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//set data to IO3 end

	//set data to IO5 begin
	f_request(SPI_WRREQ);
	HAL_SPI_Transmit(&hspi1, IO5_source, 1, 10);
	f_request(NO_REQ);
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);

	f_address(4);
	f_request(COPY_FROM_SPI);
	f_request(NO_REQ);

	//HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//set data to IO2 end

	//set data to IO6 begin
	f_request(SPI_WRREQ);
	HAL_SPI_Transmit(&hspi1, IO6_source, 1, 10);
	f_request(NO_REQ);
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);

	f_address(5);
	f_request(COPY_FROM_SPI);
	f_request(NO_REQ);

	//HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//set data to IO6 end

	//Execute test
	f_request(EXE_START);
	f_request(NO_REQ);
	HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);

	//read from IO4 to spi fifo begin
	f_address(3);
	f_request(COPY_TO_SPI);
	f_request(NO_REQ);
	HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//read from IO4 end

	//before read request, a null byte should be sent
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);
	f_request(SPI_RDREQ);
	HAL_SPI_Receive(&hspi1, IO4_capture, 2, 10);
	f_request(NO_REQ);

	//read from IO7 to spi fifo begin
	f_address(6);
	f_request(COPY_TO_SPI);
	f_request(NO_REQ);
	HAL_Delay(1);
	while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
	//read from IO7 end

	//before read request, a null byte should be sent
	HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);
	f_request(SPI_RDREQ);
	HAL_SPI_Receive(&hspi1, IO7_capture, 2, 10);
	f_request(NO_REQ);

}


void f_load_source()
{

}

void f_set_frequency(int freq)
{
	HAL_GPIO_WritePin(CLKBUF_EN_GPIO_Port, CLKBUF_EN_Pin, 1);
	clk_init();
	clk_set_frequency(0,freq);
	clk_enable(0);
}


void f_set_io_voltage(int code)
{
	setChannelValue(MCP4728_CHANNEL_A, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
			MCP4728_PD_MODE_NORMAL,true);
	setChannelValue(MCP4728_CHANNEL_B, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
			MCP4728_PD_MODE_NORMAL,true);
	setChannelValue(MCP4728_CHANNEL_C, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
			MCP4728_PD_MODE_NORMAL,true);
	setChannelValue(MCP4728_CHANNEL_D, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
			MCP4728_PD_MODE_NORMAL,true);
}


// void shiftOut8(BITORDER bitOrder, uint8_t val)
// {
// 	uint8_t i;

// 	for (i = 0; i < 8; i++)  {

// 		if (bitOrder == LSBF) {
// 			//digitalWrite(SHIFT_DATA_PIN, val & 1);
// 			HAL_GPIO_WritePin(DIN_GPIO_Port, DIN_Pin, val & 1);
// 			val = val>>1;
// 		} else {
// 			//digitalWrite(SHIFT_DATA_PIN, (val & 128) != 0);
// 			HAL_GPIO_WritePin(DIN_GPIO_Port, DIN_Pin, (val & 128) != 0);

// 			val = val<<1;


// 		}
// 		for(uint8_t j = 0; j < 4; j++){} // last delay was 4

// 		//digitalWrite(SHIFT_CLK, 1);
// 		HAL_GPIO_WritePin(SHCP_GPIO_Port, SHCP_Pin, 1);
// 		//digitalWrite(SHIFT_CLK, 0);
// 		HAL_GPIO_WritePin(SHCP_GPIO_Port, SHCP_Pin, 0);
// 	}
// }

// void f_shiftOutAll(uint8_t *io_matrix)
// {
// 	uint8_t i;
// 	for(i = 0; i < 12; i++)
// 	{
// 		shiftOut8(MSBF,io_matrix[i]);
// 	}
// 	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 0);
// 	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 1);
// 	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 0);

// 	HAL_Delay(20);
// }

// void f_matrix_init(void)
// {
// 	for(int i = 0; i < 12; i = i+4)
// 	{
// 		io_config_matrix[i] = 0b01010101;
// 		io_config_matrix[i+1] = 0b01010101;
// 		io_config_matrix[i+2] = 0;
// 		io_config_matrix[i+3] = 0;
// 	}

// 	HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, 0);
// 	HAL_GPIO_WritePin(SH_RST_GPIO_Port, SH_RST_Pin, 1);
// 	f_shiftOutAll(io_config_matrix);
// 	//HAL_Delay(1000);
// }

// void f_set_io(int device, IO_DIRECTION direction)
// {

// 	int IO_num_in_fpga = (device - 16)%16; //0,1,2,3....13,14,15
// 	int fpga_num = (device - 16)/16; //0,1,2
// 	//int relay_set_in_fpga = IO_num_in_fpga/8; //0,1

// 	//config relays
// 	uint16_t relay_binary = 1 << (15-IO_num_in_fpga);
// 	io_config_matrix[8-fpga_num*4 + 2] |= relay_binary;
// 	io_config_matrix[8-fpga_num*4 + 3] |= relay_binary>>8;

// 	//config level shifters
// 	int levelshifter_bitshift_pos[] = {8,8,10,10,14,14,12,12,2,2,0,0,6,6,4,4};

// 	uint16_t levelshifter_binary = io_config_matrix[8-fpga_num*4] | io_config_matrix[8-fpga_num * 4 + 1] <<8;
//     //cout << levelshifter_bitshift_pos[IO_num_in_fpga] <<"\n";
// 	if(direction == IO_OUTPUT)
// 	{
// 		levelshifter_binary |= 0b1<<(levelshifter_bitshift_pos[IO_num_in_fpga]+1);
//         //cout << bitset<16>(levelshifter_binary)<<"\n";
//         levelshifter_binary &=  ~( (uint16_t)(1<< (levelshifter_bitshift_pos[IO_num_in_fpga])) );
//         //cout << bitset<16>(levelshifter_binary)<<"\n";
// 	}
// /* 	else // IO INPUT
// 	{
// 		levelshifter_binary = 0b10<<(levelshifter_bitshift_pos[IO_num_in_fpga]);
// 	} */
//     //cout << bitset<16>(levelshifter_binary)<<"\n";
// 	io_config_matrix[8-fpga_num*4] = levelshifter_binary;
// 	io_config_matrix[8-fpga_num*4+1] = levelshifter_binary >>8;

// }

void f_shiftOutAll(){
	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 0);
	HAL_GPIO_WritePin(SH_RST_GPIO_Port, SH_RST_Pin, 1);
	HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, 0);
	HAL_SPI_Transmit(&hspi2, sr, 12, 100);
	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 1);
	HAL_Delay(2);
	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 0);
	HAL_Delay(100);
}


void f_matrix_init(void)
{
	for(int i = 0; i < 12; i = i+1)
	{
		sr[i] = 0; 
	}
	f_shiftOutAll();
}

void f_set_io(int io, int pinmode){
	switch(io) 
	{
		case 1 :	sr[9] |=pinmode<<0;	sr[11]|=1<<7;	break;
		case 2 :	sr[9] |=pinmode<<0;	sr[11]|=1<<6;	break;
		case 3 :	sr[9] |=pinmode<<2;	sr[11]|=1<<5;	break;
		case 4 :	sr[9] |=pinmode<<2;	sr[11]|=1<<4;	break;
		case 5 :	sr[9] |=pinmode<<6;	sr[11]|=1<<3;	break;
		case 6 :	sr[9] |=pinmode<<6;	sr[11]|=1<<2;	break;
		case 7 :	sr[9] |=pinmode<<4;	sr[11]|=1<<1;	break;
		case 8 :	sr[9] |=pinmode<<4;	sr[11]|=1<<0;	break;

		case 9 :	sr[8] |=pinmode<<2;	sr[10]|=1<<7;	break;
		case 10:	sr[8] |=pinmode<<2;	sr[10]|=1<<6;	break;
		case 11:	sr[8] |=pinmode<<0;	sr[10]|=1<<5;	break;
		case 12:	sr[8] |=pinmode<<0;	sr[10]|=1<<4;	break;
		case 13:	sr[8] |=pinmode<<6;	sr[10]|=1<<3;	break;
		case 14:	sr[8] |=pinmode<<6;	sr[10]|=1<<2;	break;
		case 15:	sr[8] |=pinmode<<4;	sr[10]|=1<<1;	break;
		case 16:	sr[8] |=pinmode<<4;	sr[10]|=1<<0;	break;

		case 17:	sr[5] |=pinmode<<0;	sr[7]|=1<<7;	break;
		case 18:	sr[5] |=pinmode<<0;	sr[7]|=1<<6;	break;
		case 19:	sr[5] |=pinmode<<2;	sr[7]|=1<<5;	break;
		case 20:	sr[5] |=pinmode<<2;	sr[7]|=1<<4;	break;
		case 21:	sr[5] |=pinmode<<6;	sr[7]|=1<<3;	break;
		case 22:	sr[5] |=pinmode<<6;	sr[7]|=1<<2;	break;
		case 23:	sr[5] |=pinmode<<4;	sr[7]|=1<<1;	break;
		case 24:	sr[5] |=pinmode<<4;	sr[7]|=1<<0;	break;

		case 25:	sr[4] |=pinmode<<2;	sr[6]|=1<<7;	break;
		case 26:	sr[4] |=pinmode<<2;	sr[6]|=1<<6;	break;
		case 27:	sr[4] |=pinmode<<0;	sr[6]|=1<<5;	break;
		case 28:	sr[4] |=pinmode<<0;	sr[6]|=1<<4;	break;
		case 29:	sr[4] |=pinmode<<6;	sr[6]|=1<<3;	break;
		case 30:	sr[4] |=pinmode<<6;	sr[6]|=1<<2;	break;
		case 31:	sr[4] |=pinmode<<4;	sr[6]|=1<<1;	break;
		case 32:	sr[4] |=pinmode<<4;	sr[6]|=1<<0;	break;

		case 33:	sr[1] |=pinmode<<0;	sr[3]|=1<<7;	break;
		case 34:	sr[1] |=pinmode<<0;	sr[3]|=1<<6;	break;
		case 35:	sr[1] |=pinmode<<2;	sr[3]|=1<<5;	break;
		case 36:	sr[1] |=pinmode<<2;	sr[3]|=1<<4;	break;
		case 37:	sr[1] |=pinmode<<6;	sr[3]|=1<<3;	break;
		case 38:	sr[1] |=pinmode<<6;	sr[3]|=1<<2;	break;
		case 39:	sr[1] |=pinmode<<4;	sr[3]|=1<<1;	break;
		case 40:	sr[1] |=pinmode<<4;	sr[3]|=1<<0;	break;

		case 41:	sr[0] |=pinmode<<2;	sr[2]|=1<<7;	break;
		case 42:	sr[0] |=pinmode<<2;	sr[2]|=1<<6;	break;
		case 43:	sr[0] |=pinmode<<0;	sr[2]|=1<<5;	break;
		case 44:	sr[0] |=pinmode<<0;	sr[2]|=1<<4;	break;
		case 45:	sr[0] |=pinmode<<6;	sr[2]|=1<<3;	break;
		case 46:	sr[0] |=pinmode<<6;	sr[2]|=1<<2;	break;
		case 47:	sr[0] |=pinmode<<4;	sr[2]|=1<<1;	break;
		case 48:	sr[0] |=pinmode<<4;	sr[2]|=1<<0;	break;
	}
}



