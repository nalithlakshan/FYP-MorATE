#include "defines.h"

//#define DEBUG_EXECUTE_COMMAND


void execute_command()
{
    uint8_t task;
    task = READ_DATA[1];

/*     Serial1.println("task: ");
    Serial1.print(task);
    breakpoint();
 */
    uint16_t value = (READ_DATA[4]<<24) | (READ_DATA[4]<<16) | (READ_DATA[4]<<8) | READ_DATA[5];

    uint16_t payload_len = READ_DATA[6]<<8 | READ_DATA[7];

    command_display(task);

    switch(task)
    {
        case(set_V_VS1):
        {
            if(PINCARD_ADDRESS == PINCARD_B)
            {
                value = (4*value/5)*50/49;
                set_power_supply(PS1,value);
            }
            else
            {
                value = 4*value/5;
                set_power_supply(PS1,value);
            }
    
            #ifdef DEBUG_EXECUTE_COMMAND 
            Serial1.println("set_V_VS1 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            break;    
        }
        case(set_V_VS2):
        {
            //set_power_supply(PS2,value);
            set_power_supply(PS2, 1000 - 100*(value-3960)/107);
            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_V_VS2 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif
            break;
        }
        case(set_V_VS3):
        {
            set_power_supply(PS3,(value/4)*(-1)+5150);
           
            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_V_VS3 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            break;
        }
        case(set_I_VS1):
        {
            set_current_limit(PS1,value);
            
            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_I_VS1 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            break;
        }
        case(set_I_VS2):
        {
            set_current_limit(PS2,value);
        
            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_I_VS2 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            break;
        }
        case(set_I_VS3):
        {
            set_current_limit(PS3,value);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_I_VS3 completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            break;
        }
        case(enable_VS1):
        {
            enable_power_supply(PS1,ON);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("enable_VS1 completed \t");
            #endif

            break;
        }
        case(enable_VS2):
        {
            enable_power_supply(PS2,ON);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("enable_VS2 completed \t");
            #endif

            break;
        }
        case(enable_VS3):
        {
            enable_power_supply(PS3,ON);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("enable_VS3 completed \t");
            #endif

            break;
        }

        case(disable_VS1):
        {
            enable_power_supply(PS1,OFF);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("disable_VS1 completed \t");
            #endif

            break;
        }
        case(disable_VS2):
        {
            enable_power_supply(PS2,OFF);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("disable_VS2 completed \t");
            #endif

            break;
        }
        case(disable_VS3):
        {
            enable_power_supply(PS3,OFF);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("disable_VS3 completed \t");
            #endif

            break;
        }

        case(VS1_pin_FB_on):
        {
            feedback_relay(PS1, DUT_PIN);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS1_pin_FB_on completed \t");
            #endif

            break;
        }
        case(VS2_pin_FB_on):
        {
            feedback_relay(PS2, DUT_PIN);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS2_pin_FB_on completed \t");
            #endif

            break;
        }
        case(VS3_pin_FB_on):
        {
            feedback_relay(PS3, DUT_PIN);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS3_pin_FB_on completed \t");
            #endif

            break;
        }
        case(VS1_pin_FB_off):
        {
            feedback_relay(PS1, IMMEDIATE);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS1_pin_FB_off completed \t");
            #endif

            break;
        }
        case(VS2_pin_FB_off):
        {
            feedback_relay(PS2, IMMEDIATE);

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS2_pin_FB_off completed \t");
            #endif

            break;
        }
        case(VS3_pin_FB_off):
        {
            feedback_relay(PS3, IMMEDIATE);
            

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("VS3_pin_FB_off completed \t");
            #endif

            break;
        }
        case(set_load_dac):
        {
            set_current_source(value);


            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("set_load_dac completed \t");
            Serial1.print("Value: ");
            Serial1.println(value);
            #endif

            
            break;
        }
        case(shift_config):
        {
            for(uint8_t i = 0; i < 21; i++ )
            {
                matrix_array[i] = READ_DATA[i+9];
            }

            shiftOutAll();

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("shift_config completed \t");
            print_switch_matrix();
            #endif

            //breakpoint();
            break;
        }
        case(measure_V):
        {
            
            buzzer_on(1);
            measurements = measure_voltage();
            update_txbuffer(measurements,measure_V);

            write_busy = 0;

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("measure_V completed \t");
            Serial1.print("Measurements: ");
            Serial1.println(measurements);
            #endif

            break;   
        }
        case(measure_I):
        {
            buzzer_on(1);
            measurements = measure_current();
            update_txbuffer(measurements,measure_I);

            write_busy = 0;

            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.println("measure_I completed \t");
            Serial1.print("Measurements: ");
            Serial1.println(measurements);
            #endif

            break;  
        }
        default: 
        {   
            #ifdef DEBUG_EXECUTE_COMMAND
            Serial1.print("Task: ");
            Serial1.println(task);
            Serial1.println("NO MATCH !!!!!!!!!!!!!!!!!!");
            #endif
        }
  
    }
    #ifdef DEBUG_EXECUTE_COMMAND
    Serial1.println("-----------------------------");
    #endif
    //breakpoint();

}


/*
typedef enum Command {set_V_VS1, set_V_VS2, set_V_VS3, set_I_VS1, set_I_VS2, set_I_VS3,
                  enable_VS1, enable_VS2, enable_VS3, disable_VS1, disable_VS2, disable_VS3,
                  VS1_pin_FB_on,VS1_pin_FB_off, VS2_pin_FB_on, VS2_pin_FB_off,
                  VS3_pin_FB_on,VS3_pin_FB_off,
                  set_load_dac, shift_config, measure_V, measure_I} Command;
*/