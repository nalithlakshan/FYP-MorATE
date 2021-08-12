#include <stdarg.h>

void print(char *msg,...){

	char buff[80];
	va_list args;
	va_start(args,msg);
	vsprintf(buff,msg,args);

	for (int i =0 ; i < strlen(buff); i++){
		//HAL_UART_Transmit(&huart1, buff[i], 1, 100);
		USART1->DR =buff[i];
		while(!(USART1->SR & USART_SR_TXE));
	}
}