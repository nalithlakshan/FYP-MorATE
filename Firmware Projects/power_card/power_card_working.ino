#include <Wire.h>
#include "defines.h"
#include "function_prototypes.h"

#define LED_D2 PC13
#define LED_D3 PC14
#define LED_D4 PC15
#define EN_1 PA0
#define ILIM_FB_1 PA1
#define ILIM_FB_2 PA2
#define LDAC_1 PA3
#define EN_2 PA4
#define RELAY_3 PA5
#define RELAY_2 PA6
#define RELAY_1 PA7
#define EN_3 PB0
#define ILIM_FB_3 PB1
#define LDAC_2 PB2
#define BUZZER PB12
#define RST PB13
#define STCP PB14
#define OE PB15
#define SHIFT_CLK PB3
#define SHIFT_DATA_PIN PB5
#define LED_D5 PB8
#define LED_D6 PB9





TwoWire Wire2 = TwoWire(PB11,PB10);

void setup() {
  // Timer for buzzer
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *MyTim = new HardwareTimer(Instance);
  MyTim->setOverflow(10, HERTZ_FORMAT); // 10 Hz
  MyTim->attachInterrupt(buzzer_callback);
  MyTim->resume();

  //IO declaration
  pinMode(LED_D2, OUTPUT);
  pinMode(LED_D3, OUTPUT);
  pinMode(LED_D4, OUTPUT);
  pinMode(EN_1, OUTPUT);
  pinMode(ILIM_FB_1, INPUT);
  pinMode(ILIM_FB_2, INPUT);
  pinMode(LDAC_1, OUTPUT);
  pinMode(EN_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(EN_3, OUTPUT_OPEN_DRAIN);
  pinMode(ILIM_FB_3, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(STCP, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(LED_D5, OUTPUT);
  pinMode(LED_D6, OUTPUT);

  //startup logic for output pins
  digitalWrite(BUZZER,0);
  digitalWrite(LED_D4,0);
  digitalWrite(LED_D4,0);
  digitalWrite(LED_D6,0);
  digitalWrite(LED_D5,1);

  //iter_num = 0;
  startup();
  Serial1.println("Started");

  //tests

/*   enable_power_supply(PS2,ON);
  set_power_supply(PS2, 1000 - 100*(3000-3960)/107);  */
/*   enable_power_supply(PS1,ON);
  set_power_supply(PS1, 4*3300/5); */

  
/*   enable_power_supply(PS3,ON);
  set_power_supply(PS3,(11500/4)*(-1)+5150); //1400 3900 (5000*(-1/4))+5150 */
/* 
  set_current_source(3000+11+8);
  matrix_array[0] = 0b00110001;
  shiftOutAll(); */

/*
//matrix_array[0]=0b00100001; // uA
matrix_array[0]=0b01000010;//mA
matrix_array[10] = 0b01001010;//A9-PS1 5V
matrix_array[18] = 0b10000100;//A3 -GND
matrix_array[17] = 0b10000100;//A4 - GND
matrix_array[7]  = 0b10000100; // A12 - GND
//A10 and A11 to PS3
matrix_array[9] = 0b00010100;
matrix_array[8] = 0b00010100;
//A1 and A2 to PS2
matrix_array[20] = 0b00100100;
matrix_array[19] = 0b00100100;
//delay(5000);
shiftOutAll(); */

/* // HC595
//matrix_array[0] = 0b00000100; // nA
//matrix_array[0]=0b01000010;//mA
matrix_array[0]=0b00100001; // uA
matrix_array[4] = 0b01000100;
matrix_array[5] = 0b10000100;
matrix_array[6] = 0b10000100;
matrix_array[7] = 0b10000100;
matrix_array[8] = 0b10000100;
matrix_array[10] = 0b01001011;
matrix_array[13] = 0b10000100;
shiftOutAll(); */

}

void loop() 
{



  read_packet_status = read_packet_from_fifo();
  //delay(100);
  if(read_packet_status) execute_command();
  delay(10);
  
  //test

/*   matrix_array[10] = 0b01000100;
  shiftOutAll();
  delay(1000);
  matrix_array[10] = 0b00000000;
  shiftOutAll();
  delay(1000); */

}


/* 
// ################### HC595 ####################
matrix_array[0] = 0b00100001;
matrix_array[4] = 0b01000100;
matrix_array[5] = 0b10000100;
matrix_array[6] = 0b10000100;
matrix_array[7] = 0b10000100;
matrix_array[8] = 0b10000100;
matrix_array[10] = 0b01001011;
matrix_array[13] = 0b10000100;
shiftOutAll(); */

/* 
for(int i = 11; i < 21; i++)
{

  for(int j = 0; j < 8; j++)
  {
    matrix_array[i] = 1<<j;
    shiftOutAll();
    delay(500);
  }
  for(uint8_t k = 0; k <21; k++)
	{
		matrix_array[k] = 0b00000000; 
	}
 delay(1000);
} */