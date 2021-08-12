#ifndef DEFINES_H
#define DEFINES_H

#define TOGGLE_STM_LED \
	HAL_GPIO_TogglePin(LED_STM_GPIO_Port, LED_STM_Pin);

//USB Communication
#define RX_BUFFER_SIZE 4096
#define TX_BUFFER_SIZE 4096
#define  FIFO_SIZE 4096  // must be 2^N
#define FIFO_INCR(x) (((x)+1)&((FIFO_SIZE)-1))

#define PIN_CARD_A_ADDR 18
#define PIN_CARD_B_ADDR 19

#define REQUEST_TIMEOUT 100 //100 milliseconds
#define PACKET_READ_TIMEOUT 10000 //10 seconds

typedef enum PIN_CARD
{
	PIN_CARD_A = PIN_CARD_A_ADDR,
	PIN_CARD_B = PIN_CARD_B_ADDR,
}PIN_CARD;


typedef enum Command {set_V_VS1, set_V_VS2, set_V_VS3, set_I_VS1, set_I_VS2, set_I_VS3,
				enable_VS1, enable_VS2, enable_VS3, disable_VS1, disable_VS2, disable_VS3,
				VS1_pin_FB_on,VS1_pin_FB_off, VS2_pin_FB_on, VS2_pin_FB_off,
				VS3_pin_FB_on,VS3_pin_FB_off,
				set_load_dac, shift_config, measure_V, measure_I, new_IC_loaded, start_reading_pincards, test_over,
				load_source_vector,read_capture_vector, execute_sourcing,reset_io_drivers,start_digital_tests,set_fpga_frequency,set_io_voltage,
				set_source_mode, set_capture_mode, bypass_clk}Command;

typedef enum Device { pincardA, pincardB, pincardC,pincardD, pincardE, pincardF, pincardG, pincardH,
				pincardI, pincardJ, pincardK, pincardL,pincardM, pincardN, pincardO, pincardP,
				io1,io2,io3,io4,io5,io6,io7,io8,io9,io10,io11,io12,io13,io14,io15,io16,
				io17,io18,io19,io20,io21,io22,io23,io24,io25,io26,io27,io28,io29,io30,io31,io32,
				io33,io34,io35,io36,io37,io38,io39,io40,io41,io42,io43,io44,io45,io46,io47,io48,io,pincards
				}Device;

#endif
