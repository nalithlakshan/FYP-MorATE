#include "setup_and_loop.h"

extern uint8_t transmit_buffer[TX_BUFFER_SIZE];
extern FIFO RX_FIFO ;
extern int totRX;

#define DEBUG_MAIN_LOOP


void setup(){
	HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 0);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(PW_CARD_RESET_GPIO_Port, PW_CARD_RESET_Pin, 1);
	HAL_Delay(1000);

	//setting up initial values for variables
	new_packet_read = 0;
	packet_sent = 0;
	num_req_A = 0;

	//fpga
	null_byte[1] = 0b00000000;
	fdata_to_ui[0] = 'S';
	fdata_to_ui[9] = 'E';
	fdata_to_ui[2] = read_capture_vector;

	f_matrix_init();
	//flags
	read_from_ui = 0;
	request_from_pincard = 0;
	notify_new_ic = 1;

/*	f_set_io(io1,IO_OUTPUT);
	f_set_io(io2,IO_OUTPUT);
	f_set_io(io3,IO_OUTPUT);
	f_set_io(io7,IO_OUTPUT);
	f_shiftOutAll(io_config_matrix);*/

	beep(3);
	//f_set_io_voltage(-2500+5300);//4000 --> 1.3V 300-->5V
}

void main_loop(){

	while(read_from_ui)
	{
		#ifdef DEBUG_MAIN_LOOP
		//print("STATE: read_from_ui\n");
		#endif
		new_packet_read = read_packet_FIFO();

		if(new_packet_read) packet_sent = decode_packet();

		#ifdef DEBUG_MAIN_LOOP
		//print("Error Code send packets: %d\n",packet_sent);
		#endif
	}

	while(request_from_pincard)
	{
		#ifdef DEBUG_MAIN_LOOP
		//print("STATE: request_from_pincard\n");
		print("num_req_A: %d\n",num_req_A);
		#endif

		request_from_pin_card();
		HAL_Delay(100);
	}


	while(notify_new_ic)
	{
		#ifdef DEBUG_MAIN_LOOP
		//print("STATE: notify_new_ic\n");
		#endif
		if(HAL_GPIO_ReadPin(new_ic_switch_GPIO_Port, new_ic_switch_Pin) == 0)
		{
			notify_new_ic = 0;
			read_from_ui = 1;
			header_to_ui[1] = pincards;
			header_to_ui[2] = new_IC_loaded;

			while(CDC_Transmit_FS(header_to_ui, 10));
		}
	}



}





uint32_t VCP_read(uint8_t* Buf, uint32_t Len)
{
  uint32_t count=0;
  /* Check inputs */
  if ((Buf == NULL) || (Len == 0))
  {
    return 0;
  }

  while (Len--)
  {
    if (RX_FIFO.head==RX_FIFO.tail) return count;
    count++;
    *Buf++=RX_FIFO.data[RX_FIFO.tail];
    RX_FIFO.tail=FIFO_INCR(RX_FIFO.tail);
  }
  return count;
}


void print(char *msg,...){
  char buff[80];
  va_list args;
  va_start(args,msg);
  vsprintf(buff,msg,args);
  //while(CDC_Transmit_FS(buff, strlen(buff)));
	for (int i =0 ; i < strlen(buff); i++){
		//HAL_UART_Transmit(&huart1, buff[i], 1, 100);
		USART1->DR =buff[i];
		while(!(USART1->SR & USART_SR_TXE));
	}
}


void beep(int duration)
{
	buzzerTime = duration;
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
}


void led_kinghtrider(){
	//	LED Testing Note: LED3 not working!
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, 0);
	HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, 0);
	HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, 0);
	HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, 0);
	HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, 0);
	HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, 1);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, 0);
}

/*
	#ifdef DEBUG_MAIN_LOOP
	if( (HAL_GetTick()-lt_packet) > PACKET_READ_TIMEOUT )
	{
		print("NO PACKETS FROM UI\n");
	}
	#endif

	new_packet_read = read_packet_FIFO();

	#ifdef DEBUG_MAIN_LOOP
	print("Read Packet from RX_FIFO status:%d\n",new_packet_read);
	print("Number of Packets from UI: %d\n",num_packets_ui);
	#endif

	if(new_packet_read)	packet_sent = send_packet();

	#ifdef DEBUG_MAIN_LOOP
	print("Packet sending ERROR_CODE: %d\n",packet_sent);
	#endif

	request_A = ( (num_req_A > 0) );
	request_B = ( (num_req_B > 0) );
	//print("time gap: %d\n",time_gap);
	print("Requests A: %d\tRequests B: %d\n",num_req_A, num_req_B);

	if( request_A || request_B)

	{

		request_from_pin_card(); //last request times will be updated inside this

		request_A = ( (num_req_A > 0) );
		request_B = ( (num_req_B > 0) );
		HAL_Delay(50);
	}

	#ifdef DEBUG_MAIN_LOOP
	print("-----------------------------------\n");
	#endif

	HAL_Delay(100);

*/
