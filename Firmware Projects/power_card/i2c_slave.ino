#include "defines.h"

//#define DEBUG_READ_PACKET_FROM_FIFO
//#define DEBUG_UPDATE_TX_BUFFER
//#define DEBUG_ON_REQUEST_MASTER

void receiveEvent(int num_bytes)
{   
    //Serial1.println(Wire.available());

    while(Wire.available()>0)
    {
        RX_FIFO.data[RX_FIFO.head] = Wire.read();
        RX_FIFO.head = RX_FIFO_INCR(RX_FIFO.head);
        //break;
    }
}



/*    ----------------------------------------------------------
      |  0   |    1      |   2 3 4 5  |    6 7          |   8   |
      ----------------------------------------------------------
      |  S   |  COMMAND  |  VALUE     |   PAYLOAD_SIZE  |   E   |
      ----------------------------------------------------------    
*/

uint8_t read_packet_from_fifo()
{
    if(RX_FIFO.head == RX_FIFO.tail) return 0;

    read_id = 0;
    read_len = 9;

    #ifdef DEBUG_READ_PACKET_FROM_FIFO
    Serial1.println("Packet reading started");
    #endif

    while(read_id < read_len)
    {

        //මේක නැතුව වැඩ කරන්නේ  නෑ
        delay(3); 

        if(RX_FIFO.tail != RX_FIFO.head)
        {
            READ_DATA[read_id] = RX_FIFO.data[RX_FIFO.tail];
            RX_FIFO.tail = RX_FIFO_INCR(RX_FIFO.tail);

            if(read_id == 8) // packet is read
            {
            payload_size = READ_DATA[6]<<8 | READ_DATA[7]; // read the payload_size
            
                if(payload_size != 0) //switch matrix will be received
                {
                    read_len += payload_size; //extend the read length
                
                }
                //else read_id will exceed the read_len
            }
            read_id++;
        }
        else knight_rider();

    }
    #ifdef DEBUG_READ_PACKET_FROM_FIFO
    if(READ_DATA[0] != 'S') buzzer_on(2);
    Serial1.println("Packet reading completed");
    #endif

    return 1;
}

void on_request_master()
{
    #ifdef DEBUG_ON_REQUEST_MASTER
    Serial1.println("ENTERED REQUEST HANDLER");
    Serial1.print("write_busy: ");
    Serial1.println(write_busy);
    #endif

    if(write_busy == 1)
    {
        Wire.write(BUSY_BUFFER,10);
    }
    else
    {
        #ifdef DEBUG_ON_REQUEST_MASTER
        Serial1.print("Head: ");
        Serial1.print(TX_FIFO.head);
        Serial1.print("  ");
        Serial1.print("Tail: ");
        Serial1.print(TX_FIFO.tail);
        Serial1.println();
        #endif

        if(TX_FIFO.head != TX_FIFO.tail)
        {
            //send the entire packet
            uint8_t tx_packet[10];
            
            for(uint8_t i = 0; i < 10; i++) // increment tail by a packet size
            {
                tx_packet[i] = TX_FIFO.data[TX_FIFO.tail];
                TX_FIFO.tail = TX_FIFO_INCR(TX_FIFO.tail);
            }
            Wire.write(tx_packet,10);
        }
        else
        {
            Wire.write(BUSY_BUFFER,10);
        }

        if(TX_FIFO.head == TX_FIFO.tail)
        {
            write_busy = 1;
        }

        #ifdef DEBUG_ON_REQUEST_MASTER
        Serial1.print("Head: ");
        Serial1.print(TX_FIFO.head);
        Serial1.print("  ");
        Serial1.print("Tail: ");
        Serial1.print(TX_FIFO.tail);
        Serial1.println();
        #endif

    }
    
    #ifdef DEBUG_ON_REQUEST_MASTER
    Serial1.println("EXIT REQUEST HANDLER");
    Serial1.print("write_busy: ");
    Serial1.println(write_busy);
    #endif
    

}


    /*    ----------------------------------------------------------
      |  0   |   1   |    2      |   3 4 5 6  |    7 8          |   9   |
      ----------------------------------------------------------
      |  S   |   ID  |  COMMAND  |  VALUE     |   PAYLOAD_SIZE  |   E   |
      ----------------------------------------------------------    
*/
void update_txbuffer(int measurements, Command command)
{
    //S
    TX_FIFO.data[TX_FIFO.head] = 'S';
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);
    
    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif
    //DEVICE_ID
    TX_FIFO.data[TX_FIFO.head] = DEVICE_ID;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);

    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif
    //COMMAND
    TX_FIFO.data[TX_FIFO.head] = command;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);

    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif
    //VALUE         0 | 0  | value[1] |  value[0]
    TX_FIFO.data[TX_FIFO.head] =0;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);
    TX_FIFO.data[TX_FIFO.head] =0;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);
    TX_FIFO.data[TX_FIFO.head] = (uint8_t)((measurements>>8) & 0x000000ff);
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);
    TX_FIFO.data[TX_FIFO.head] = (uint8_t)(measurements & 0x000000ff);
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);

    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif
    //payload_size
    TX_FIFO.data[TX_FIFO.head] =0;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);
    TX_FIFO.data[TX_FIFO.head] =0;
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);

    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif
    //E
    TX_FIFO.data[TX_FIFO.head] = 'E';
    TX_FIFO.head = TX_FIFO_INCR(TX_FIFO.head);  

    #ifdef DEBUG_UPDATE_TX_BUFFER
    Serial1.print("Head: ");
    Serial1.print(TX_FIFO.head);
    Serial1.print("\t");
    Serial1.print("Tail: ");
    Serial1.print(TX_FIFO.tail);
    Serial1.println();
    #endif 
}


