/*
 * pin_card.c
 *
 *  Created on: Jan 18, 2021
 *      Author: Thilina
 */
#include "pin_card.h"
#define DEBUG_REQUEST_FROM_PIN_CARD

void request_from_pin_card(){
    /*    ----------------------------------------------------------
      |  0   |   1   |    2      |   3 4 5 6  |    7 8          |   9   |
      ----------------------------------------------------------
      |  S   |   ID  |  COMMAND  |  VALUE     |   PAYLOAD_SIZE  |   E   |
      ----------------------------------------------------------
*/
	//request from PIN_CARD_A
	if(num_req_A > 0)
	{
		request_status = HAL_I2C_Master_Receive(&hi2c1, PIN_CARD_A_ADDR<<1, header_to_ui, 10, 100);
		header_to_ui[1] = pincardA;
		if((header_to_ui[2] == measure_I)||(header_to_ui[2] == measure_V))
		{
			while(CDC_Transmit_FS(header_to_ui, 10));
			num_req_A --;
			//print("Measurement Received\n");
		}
		else if(header_to_ui[2] == 0xAA)
		{
			//print("Slave BUSY\n");
		}
		else if(header_to_ui[2] == 0xBB)
		{
			//print("SLAVE BUFFER EMPTY");
		}

		print("REQUEST status PINCARD_A %d\n",request_status);
		for(int i = 0; i < 10; i++)
		{
			print("%d ",header_to_ui[i]);
		}
		print("\n");

	}
	//Requests from pincard B
	else if(num_req_B > 0)
	{
		request_status = HAL_I2C_Master_Receive(&hi2c1, PIN_CARD_B_ADDR<<1, header_to_ui, 10, 100);
		header_to_ui[1] = pincardB;
		if((header_to_ui[2] == measure_I)||(header_to_ui[2] == measure_V))
		{
			while(CDC_Transmit_FS(header_to_ui, 10));
			num_req_B --;
			//print("Measurement Received\n");
		}
		else if(header_to_ui[2] == 0xAA)
		{
			//print("Slave BUSY\n");
		}
		else if(header_to_ui[2] == 0xBB)
		{
			//print("SLAVE BUFFER EMPTY");
		}

		print("REQUEST status PINCARD_A %d\n",request_status);
		for(int i = 0; i < 10; i++)
		{
			print("%d ",header_to_ui[i]);
		}
		print("\n");

	}
	else
	{
		notify_new_ic = 0;
		request_from_pincard = 0;
		read_from_ui = 1;

	}

	//request from PIN_CARD_B
/*	if(request_B)
	{
		lrt_B = crt_B;
		request_status = HAL_I2C_Master_Receive(&hi2c1, PIN_CARD_B_ADDR<<1, collected_data_B, 10, 100);

		if(collected_data_B[2]!=0)
		{
			while(CDC_Transmit_FS(collected_data_B, 10));
			num_req_B --;
		}
	}*/

}

uint8_t decode_packet()
{
	/*    --------------------------------------------------------------
			  |  0   | 1  |    2    |   3 4 5 6  |     7 8         |   9   |
			  --------------------------------------------------------------
			  |  S   | ID |COMMAND  |  VALUE     |   PAYLOAD_SIZE  |   E   |
			  ---------------------------------------------------------------*/

	//device, command, value, payload_size

	uint8_t e = 0; // error flag

	if(packet[1] == pincardA) // To PINCARD_A
	{

		packet[1] = packet[0];

		e = HAL_I2C_Master_Transmit(&hi2c1, PIN_CARD_A_ADDR<<1, &packet[1], packet_size -1, 100);
		HAL_Delay(1);

		//counting measure packets
		if((packet[2] == measure_V) || (packet[2] == measure_I))
		{
			num_req_A ++;
		}
	}
	else if(packet[1] == pincardB) // To PINCARD_B
	{
		packet[1] = packet[0];
		e = HAL_I2C_Master_Transmit(&hi2c1, PIN_CARD_B_ADDR<<1, &packet[1], packet_size-1, 100);
		//HAL_I2C_Master_Transmit(&hi2c1, PIN_CARD_B_ADDR<<1, &packet[2], packet_size - 2, 100);
		HAL_Delay(1);

		//counting measure packets
		if((packet[2] == measure_V) || (packet[2] == measure_I))
		{
			num_req_B ++;
		}
	}

	else if(device == io)
	{
		if( command == start_digital_tests)
		{
			//###################
			beep(3);
		}

		else if(command == execute_sourcing)
		{
			//configure relay and level shifters
			f_shiftOutAll();

			//Execute test
			f_request(EXE_START);
			f_request(NO_REQ);
			HAL_Delay(1);
			while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);

			//reset relay and level shifters
			f_matrix_init();
		}
		else if( command == reset_io_drivers)
		{
			f_reset();

		}
		else if(command == set_fpga_frequency)
		{
			f_set_frequency(value);
		}
		else if(command = set_io_voltage )
		{
			f_set_io_voltage(-value+5300);
		}


	}

	else if( (device >= io1) && (device <= io48) )
	{
		if(device <= io16) f_cs(1);
		else if(device <= io32) f_cs(2);
		else if(device <= io48) f_cs(3);

		if(command == load_source_vector)
		{
			//set data to iox
			f_request(SPI_WRREQ);
			HAL_SPI_Transmit(&hspi1, &packet[10], payload_size, HAL_MAX_DELAY);
			f_request(NO_REQ);
			HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);

			f_address(device - 16); // io1 --> address 0
			f_request(COPY_FROM_SPI);
			f_request(NO_REQ);
			//HAL_Delay(1);
			while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);
		}

		else if(command == read_capture_vector)
		{
			//read from iox to spi fifo begin
			f_address(device-16);
			f_request(COPY_TO_SPI);
			f_request(NO_REQ);
			HAL_Delay(1);
			while(HAL_GPIO_ReadPin(F_REQ_DONE_GPIO_Port, F_REQ_DONE_Pin)== 0);

			//before read request, a null byte should be sent
			HAL_SPI_Transmit(&hspi1, null_byte, 1, 10);
			f_request(SPI_RDREQ);
			HAL_SPI_Receive(&hspi1, &fdata_to_ui[10], value, HAL_MAX_DELAY);
			f_request(NO_REQ);

			fdata_to_ui[1] = device;
			fdata_to_ui[7] = (value>>8);
			fdata_to_ui[8] = value;

			while(CDC_Transmit_FS(fdata_to_ui, value+10));
		}
		else if(command == set_source_mode)
		{
			f_set_io(device-15, 1);
		}
		else if(command == set_capture_mode)
		{
			f_set_io(device-15, 0);
		}
		else if(command == bypass_clk)
		{
			f_request(CLOCK_BYPASS);
			f_request(NO_REQ);
			f_address(device - 16);
			f_request(NO_REQ);
			f_set_io(device-15, 1);

		}
	}

	else if((packet[1] == pincards) && (packet[2]==start_reading_pincards))
	{
		read_from_ui = 0;
		notify_new_ic = 0;

		request_from_pincard = 1;

	}

	else if((packet[1] == pincards) && (packet[2] == test_over))
	{
		print("TEST OVER\n");
		notify_new_ic = 1;
		read_from_ui = 0;
		request_from_pincard = 0;
		beep(20);
/*		HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 0);
		HAL_Delay(1000);
		HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 1);
		HAL_Delay(1000);*/

	}


	if(e != 0)
	{
		beep(100);
		HAL_Delay(200);
		beep(50);
		HAL_Delay(200);
		beep(100);
		HAL_Delay(200);
		beep(50);
		HAL_Delay(200);
	}

	return e;
}



// return 0 if the fifo is empty
//returns 1 for a successfull read
uint8_t read_packet_FIFO()
{

	if(RX_FIFO.head == RX_FIFO.tail) return 0;

	read_id = 0;
	packet_size =10;

	print("Read packet from FIFO started\n");
	while(read_id < packet_size)
	{
		if(RX_FIFO.tail != RX_FIFO.head)
		{
		  packet[read_id] = RX_FIFO.data[RX_FIFO.tail];
		  RX_FIFO.tail = FIFO_INCR(RX_FIFO.tail);

		  if(read_id == 9) // packet is read
		  {
		    device  = packet[1];
			command = packet[2];
			value   = (packet[3]<<24)|(packet[4]<<16)|(packet[5]<<8)|(packet[6]);
			payload_size = (packet[7]<<8) | (packet[8]); // read the payload_size

//			print("device = %d  command = %d   value = %d   payload_size = %d \n",device,command,value,payload_size);

			if(payload_size != 0) //switch matrix will be received
			{
			  packet_size = packet_size + payload_size; //extend the read length
			}
			//else read_id will exceed the packet_size
		  }

		  read_id++;
		}

		else led_kinghtrider();

	}

	lt_packet = HAL_GetTick();
	print("Read packet from FIFO ended\n");
	return 1;
}











/*		int num_retransmit = 0;

		while((e != 0) && (num_packets_A == 0) && (num_retransmit < 3)) // if not acknowledged, reset the pin card A
		{
			if(e != 0)
			{
				beep(100);
				HAL_Delay(200);
				beep(50);
				HAL_Delay(200);
				beep(100);
				HAL_Delay(200);
				beep(50);
				HAL_Delay(200);
			}

			print("Pin Card not responding. Retransmitting....\n");

			HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 0);
			HAL_Delay(1000);
			HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 1);
			HAL_Delay(1000);

			e = HAL_I2C_Master_Transmit(&hi2c1, PIN_CARD_A_ADDR<<1, &packet[1], packet_size -1, 100);
			HAL_Delay(1);
			num_retransmit += 1;
		}

		if((e == 0) && num_packets_A == 0)
		{
			print("First packet successfully sent to PINCARD_A\n");
		}
		else if((e != 0) && num_packets_A == 0)
		{
			print("FIRST PACKET RETRANSMISSION FAILED\n");
		}
		else if((e==0) && num_packets_A != 0)
		{
			print("Packet number %d to PINCARD_A sent.",num_packets_A);
		}
		else if((e!=0) && num_packets_A != 0)
		{
			print("Packet number %d to PINCARD_A failed.", num_packets_A);
		}*/
