
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "defines.h"



void set_power_supply(PS select, uint16_t code )
{		
        if(select == 1)
        {
	
        	setChannelValue(0x60,MCP4728_CHANNEL_B, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
        			MCP4728_PD_MODE_NORMAL,true);
        }
        else if(select == 2)
        {
        	setChannelValue(0x60,MCP4728_CHANNEL_D, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
            			MCP4728_PD_MODE_NORMAL,true);
        }
        else if(select == 3)
        {
        	setChannelValue(0x64,MCP4728_CHANNEL_A, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
            			MCP4728_PD_MODE_NORMAL,true);
        }
}

void enable_power_supply(PS select, EN enable)
{
	if(select == 1)
	{
		if(enable) digitalWrite(EN_1, 1);
		else digitalWrite(EN_1, 0);

	}
	else if(select == 2)
	{
		if(enable) digitalWrite(EN_2, 1);
		else digitalWrite(EN_2, 0);
	}
	else if(select == 3)
	{
		if(enable) digitalWrite(EN_3, 1);
		else digitalWrite(EN_3, 0);
	}
}

void feedback_relay(PS select, RELAY feedback)
{
	if(select == 1) digitalWrite(RELAY_3, feedback);
	else if(select == 2) digitalWrite(RELAY_2, feedback);
	else if(select == 3) digitalWrite(RELAY_1, feedback);
}

void set_current_limit(PS select, uint16_t code)
{
	if(select == 1)
	{
		setChannelValue(0x60,MCP4728_CHANNEL_A, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
            			MCP4728_PD_MODE_NORMAL,true);
	}
	else if(select == 2)
	{
		setChannelValue(0x60,MCP4728_CHANNEL_C, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
            			MCP4728_PD_MODE_NORMAL,true);
	}
	else if(select == 3)
	{
		setChannelValue(0x64,MCP4728_CHANNEL_B, code,MCP4728_VREF_INTERNAL,MCP4728_GAIN_1X,
            			MCP4728_PD_MODE_NORMAL,true);
	}
}

//handler for current limit

void I_lim_trigger_PS1()
{
	I_lim_flag_PS1 = 1;
}

void I_lim_trigger_PS2()
{
	I_lim_flag_PS2 = 1;
}

void I_lim_trigger_PS3()
{
	I_lim_flag_PS3 = 1;
}