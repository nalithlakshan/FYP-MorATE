#pragma once
#include <mcp4728.h>
#include <stdbool.h>
#include "i2c.h"
bool setChannelValue(
    MCP4728_channel_t channel, uint16_t new_value, MCP4728_vref_t new_vref,
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

  uint8_t st=HAL_I2C_Master_Transmit(&hi2c2, 0x60<<1, output_buffer, 3, 1);

  return st;
}


// equation  x - DAC voltage y-LDO output voltage
//y = -1.9991x + 4.8557
