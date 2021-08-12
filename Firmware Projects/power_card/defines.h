#ifndef DEFINES_H
#define DEFINES_H
//################## Power Supplies ###########################
typedef enum supply
{
	PS1 = 1,
	PS2 = 2,
	PS3 = 3,
}PS;

typedef enum enable
{
	OFF = 0,
	ON = 1,
}EN;

typedef enum relay
{
	DUT_PIN = 0,
	IMMEDIATE = 1,
}RELAY;

volatile uint8_t I_lim_flag_PS1;
volatile uint8_t I_lim_flag_PS2;
volatile uint8_t I_lim_flag_PS3;


//################# ADC ########################
typedef enum adc_channel
{
  CHANNEL0 = 0,
  CHANNEL1 = 1,
  CHANNEL2 = 2,
  CHANNEL3 = 3,
}ADC_CHANNEL;

typedef enum adc_select
{
  ADC_CURRENT, //
  ADC_VOLTAGE,
}ADC_SELECT;

//################ DAC #############################

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
// typedef channel MCP4728_channel_t;


//################### Switch Matrix #########################
typedef enum bitorder
{
  LSBF = 0,
  MSBF = 1,
}BITORDER;

uint8_t matrix_array[21];

uint8_t iter_num;

//################### i2c_slave ##############################
#define PINCARD_A 18
#define PINCARD_B 19

#define PINCARD_ADDRESS PINCARD_A

#define  RX_FIFO_SIZE 4096  // must be 2^N
#define RX_FIFO_INCR(x) (((x)+1)&((RX_FIFO_SIZE)-1))

typedef struct RECEIVE_FIFO
{
	uint32_t head;
	uint32_t tail;
  uint8_t data[RX_FIFO_SIZE];
} RECEIVE_FIFO;

RECEIVE_FIFO RX_FIFO = {.head=0, .tail=0};

int read_id;
int read_len;
uint8_t switch_matrix_arrived;
uint16_t payload_size;

uint8_t READ_DATA[30];

#define  TX_FIFO_SIZE 512  // must be 2^N
#define TX_FIFO_INCR(x) (((x)+1)&((TX_FIFO_SIZE)-1))

typedef struct TRANSMIT_FIFO
{
	uint32_t head;
	uint32_t tail;
  uint8_t data[TX_FIFO_SIZE];
} TRANSMIT_FIFO;

TRANSMIT_FIFO TX_FIFO = {.head = 0, .tail = 0};

uint8_t BUSY_BUFFER[10];
uint8_t write_busy;

#define DEVICE_ID 0

uint8_t read_packet_status;
//uint8_t last_measurement_sent;

// ##################  Commands from Main MCU #######################
typedef enum Command {set_V_VS1, set_V_VS2, set_V_VS3, set_I_VS1, set_I_VS2, set_I_VS3,
                  enable_VS1, enable_VS2, enable_VS3, disable_VS1, disable_VS2, disable_VS3,
                  VS1_pin_FB_on,VS1_pin_FB_off, VS2_pin_FB_on, VS2_pin_FB_off,
                  VS3_pin_FB_on,VS3_pin_FB_off,
                  set_load_dac, shift_config, measure_V, measure_I} Command;

int measurements;




#endif

