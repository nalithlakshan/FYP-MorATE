#include "defines.h";
void breakpoint()
 {
  while(1)
  {
    if(Serial1.available() > 0)
    {
      if(Serial1.read() == 'a')
      {
        break;
      }
    }
  }
 }

 void debug()
 {
     
     Serial1.println("READ_DATA: ");
     Serial1.print(READ_DATA[read_id]);

     Serial1.println("head: ");
     Serial1.print(RX_FIFO.head);

     Serial1.println("tail: ");
     Serial1.print(RX_FIFO.tail);

     Serial1.println("read_id: ");
     Serial1.print(read_id);

     Serial1.println("------------------------------------------");

 }

 void print_switch_matrix()
 {
   Serial1.println("Switch Matrix");
   Serial1.println();
   for(uint8_t i = 0; i < 21; i++)
   {
     Serial1.print("Row ");
     Serial1.print(i);
     Serial1.print("\t\t");
     Serial1.println(matrix_array[i],BIN);
   }
 }
 
void knight_rider()
{
  digitalWrite(LED_D4,1);
  delay(50);

  digitalWrite(LED_D4,0);
  digitalWrite(LED_D3,1);
  delay(50);

  digitalWrite(LED_D3,0);
  digitalWrite(LED_D2,1);
  delay(50);

  digitalWrite(LED_D2,0);
  digitalWrite(LED_D6,1);
  delay(50);

  digitalWrite(LED_D6,0);
  digitalWrite(LED_D5,1);
  delay(50);

  digitalWrite(LED_D5,0);
 
}

void command_display(uint8_t task)
{
  int bit0 = (task & 0b1)>0? 1:0;
	int bit1 = (task & 0b10)>0? 1:0;
	int bit2 = (task & 0b100)>0? 1:0;
	int bit3 = (task & 0b1000)>0? 1:0;
  int bit4 = (task & 0b10000)>0? 1:0;

  digitalWrite(LED_D4,bit0);
  digitalWrite(LED_D3,bit1);
  digitalWrite(LED_D2,bit2);
  digitalWrite(LED_D6,bit3);
  digitalWrite(LED_D5,bit4);

}