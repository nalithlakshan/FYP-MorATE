#ifndef FUNCTION_PROTOTYPES_H
#define FUNCTION_PROTOTYPES_H

//########## power supply #######################
void set_power_supply(PS select, uint16_t code );
void enable_power_supply(PS select, EN enable);
void feedback_relay(PS select, RELAY feedback);
void set_current_limit(PS select, uint16_t code);
void I_lim_trigger_PS1();
void I_lim_trigger_PS2();
void I_lim_trigger_PS2();

//########## ADC ################################
int16_t adc_read(ADC_SELECT select,ADC_CHANNEL channel);
int measure_voltage();
int measure_current();
int sort_ascend(const void *cmp1, const void *cmp2);

//########### print ###########################
void print(char *msg,...);

//######## DAC set_current_source #####################
void set_current_source(uint16_t code);

//############# i2c_slave_callback ######################
void receiveEvent(int num_bytes);


// ############## util ###########################
 void breakpoint();
 void debug();
 void print_switch_matrix();

//############## command #########################
void execute_command();


// ################## startup #######################
void startup();
#endif