#include <stdint.h>
#include <stdbool.h>
#include <Wire.h>
#include "defines.h"

// void setChannelValue(
//     uint8_t address,MCP4728_channel_t channel, uint16_t new_value, MCP4728_vref_t new_vref,
//     MCP4728_gain_t new_gain, MCP4728_pd_mode_t new_pd_mode, bool udac);


//########################### function declaration ##############################################


void setChannelValue(
    uint8_t address,MCP4728_channel_t channel, uint16_t new_value, MCP4728_vref_t new_vref,
    MCP4728_gain_t new_gain, MCP4728_pd_mode_t new_pd_mode, bool udac) {

  uint8_t output_buffer[3];

  // build the setter header/ "address"
  // 0 1 0 0 0 DAC1 DAC0 UDAC[A]
  uint8_t sequential_write_cmd = MCP4728_MULTI_IR_CMD;
  sequential_write_cmd |= (channel << 1);
  sequential_write_cmd |= udac;

  output_buffer[0] = sequential_write_cmd;
  // VREF PD1 PD0 Gx D11 D10 D9 D8 [A] D7 D6 D5 D4 D3 D2 D1 D0 [A]
  new_value |= (new_vref << 15);
  new_value |= (new_pd_mode << 13);
  new_value |= (new_gain << 12);

  output_buffer[1] = new_value >> 8;
  output_buffer[2] = new_value & 0xFF;

  Wire2.beginTransmission(address);
  Wire2.write(output_buffer,3);
  Wire2.endTransmission();

  //HAL_I2C_Master_Transmit(&hi2c2,address<<1 , output_buffer, 3, 1);
  
}

void set_current_source(uint16_t code) //channel D is used; channel D is idle
{
        	setChannelValue(0x64,MCP4728_CHANNEL_D, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_2X,
            			MCP4728_PD_MODE_NORMAL,true);
}

