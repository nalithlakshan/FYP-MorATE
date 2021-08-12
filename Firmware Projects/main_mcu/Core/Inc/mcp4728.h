#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MCP4728_I2CADDR_DEFAULT 0x60 ///< MCP4728 default i2c address

#define MCP4728_MULTI_IR_CMD                                                   \
  0x40 ///< Command to write to the input register only
#define MCP4728_MULTI_EEPROM_CMD                                               \
  0x50 ///< Command to write to the input register and EEPROM
#define MCP4728_FAST_WRITE_CMD                                                 \
  0xC0 ///< Command to write all channels at once with

/* *
 * @brief Power status values
 *
 * Allowed values for `setPowerMode`.
 */

typedef enum pd_mode {
  MCP4728_PD_MODE_NORMAL, ///< Normal; the channel outputs the given value as
                          ///< normal.
  MCP4728_PD_MODE_GND_1K, ///< VOUT is loaded with 1 kΩ resistor to ground. Most
                          ///< of the channel circuits are powered off.
  MCP4728_PD_MODE_GND_100K, ///< VOUT is loaded with 100 kΩ resistor to ground.
                            ///< Most of the channel circuits are powered off.
  MCP4728_PD_MODE_GND_500K, ///< VOUT is loaded with 500 kΩ resistor to ground.
                            ///< Most of the channel circuits are powered off.
} MCP4728_pd_mode_t;

/* *
 * @brief Example enum values
 *
 * Allowed values for `setGain`.
 */
typedef enum gain {
  MCP4728_GAIN_1X,
  MCP4728_GAIN_2X,
} MCP4728_gain_t;

/* *
 * @brief Ex
 *
 * Allowed values for `setVref`.
 */
typedef enum vref {
  MCP4728_VREF_VDD,
  MCP4728_VREF_INTERNAL,
} MCP4728_vref_t;

/* *
 * @brief Example enum values
 *
 * Allowed values for `setChannelGain`.
 */
typedef enum channel {
  MCP4728_CHANNEL_A,
  MCP4728_CHANNEL_B,
  MCP4728_CHANNEL_C,
  MCP4728_CHANNEL_D,
} MCP4728_channel_t;

/* !
 *    @brief  Class that stores state and functions for interacting with
 *            the MCP4728 I2C Digital Potentiometer
 */
bool setChannelValue(MCP4728_channel_t channel, uint16_t new_value,
                       MCP4728_vref_t new_vref,
                       MCP4728_gain_t new_gain ,
                       MCP4728_pd_mode_t new_pd_mode  ,
                       bool udac );