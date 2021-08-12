#include "defines.h"
#include "function_prototypes.h"

void startup()
{

    //disable power supply
    enable_power_supply(PS1,OFF);
    enable_power_supply(PS2,OFF);
    enable_power_supply(PS3,OFF);

    /*    ----------------------------------------------------------
      |  0   |   1   |    2      |   3 4 5 6  |    7 8          |   9   |
      ----------------------------------------------------------
      |  S   |   ID  |  COMMAND  |  VALUE     |   PAYLOAD_SIZE  |   E   |
      ----------------------------------------------------------    
*/
    // preprocess the constant values of buffer writing to master;
    BUSY_BUFFER[0] = 'S';
    BUSY_BUFFER[1] = DEVICE_ID;
    BUSY_BUFFER[3] = 0;
    BUSY_BUFFER[4] = 0;
    BUSY_BUFFER[7] = 0;
    BUSY_BUFFER[8] = 0;
    BUSY_BUFFER[9] = 'E';

    //device busy flag
    write_busy = 1;

    // switch matrix intitialization
    matrix_init();

    //i2c2 initialize
    Wire2.begin();
    Serial.begin(19200);
    Serial1.begin(19200);

    //i2c interrupt
    Wire.begin(PINCARD_ADDRESS);                // join i2c bus with address #18
    Wire.onReceive(receiveEvent); // register event
    Wire.onRequest(on_request_master);

    read_id = 0;
    read_len = 9;
    read_packet_status = 0;

    buzzer_on(1);
    delay(200);
    buzzer_on(1);
    delay(200);
    buzzer_on(3);
  
}