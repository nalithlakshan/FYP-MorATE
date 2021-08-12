#include <Wire.h>

#include "defines.h"

//uint16_t adc_read(ADC_SELECT select,ADC_CHANNEL channel);

//################# Function Declaration ###########################

int16_t adc_read(ADC_SELECT select,ADC_CHANNEL channel)
{
	int16_t result;
	uint8_t address;
	uint8_t buffer[6];
	buffer[0] = 0x01;

	if(select == 0)address = 0b1001000; //current
	else address =0b1001001; // voltage
	buffer[2] = 0b10000011; //default configs for LSB of config reg
	//buffer[1] = 0b10000010;//Gain 4.096, continuous conversion
	if(select == 1)
	{
		switch(channel)//all channels are measured wrt GND
		{
		case(0): buffer[1] = 0b11000010;
				break;
		case(1): buffer[1] = 0b11010010;
				break;
		case(2): buffer[1] = 0b11100010;
				break;
		case(3): buffer[1] = 0b11110010;
				break;
		}
	}
	else
	{
		switch(channel)//all channels are measured wrt GND
		{
		case(0): buffer[1] = 0b10000010;//channel0 wrt to channel1
				break;
		case(1): buffer[1] = 0b11010010;
				break;
		case(2): buffer[1] = 0b11100010;
				break;
		case(3): buffer[1] = 0b11110010;
				break;
		}
	}


	Wire2.beginTransmission(address);
	Wire2.write(buffer,3);
	Wire2.endTransmission();
	buffer[0] = 0x00;

	Wire2.beginTransmission(address);
	Wire2.write(buffer,1);
	Wire2.endTransmission();

	delay(20);

	Wire2.requestFrom(address,2);

	buffer[0]= Wire2.read();
	buffer[1]= Wire2.read();


	result = (buffer[0]<<8 | buffer[1]);
	return result;

}

//voltage channel 1
//current channel 0
int measure_voltage()
{
	int n_samples = 10;
	int good_samples = 4;
	int16_t samples[n_samples] ; // {1510,1511,1490,1508,1511,1499,1500,1506,1506,1507};
	for(uint8_t i = 0; i < n_samples; i++)
	{
		samples[i] = adc_read(ADC_VOLTAGE, CHANNEL1);
		Serial1.println(samples[i]);
	}

	qsort(samples,10,sizeof(samples[0]), sort_ascend);

/* 	for(int i = 0; i < n_samples; i++)
	{
		Serial1.print(samples[i]);
		Serial1.print(" ");
	} */
	
	int remove = (n_samples-good_samples)/2;
	int result = 0;

	for(uint8_t i = remove ; i < n_samples-remove; i++ )
	{
		result += samples[i];
		
	}

/* 	Serial1.println((uint16_t)(result/(n_samples-2*remove)));
	
	breakpoint(); */
	result = (result/(n_samples-2*remove));
	return result;


	
}

int measure_current()
{
	
	int n_samples = 10;
	int good_samples = 4;
	int16_t samples[n_samples] ; // {1510,1511,1490,1508,1511,1499,1500,1506,1506,1507};
	for(uint8_t i = 0; i < n_samples; i++)
	{
		samples[i] = adc_read(ADC_CURRENT, CHANNEL0);
	}

	qsort(samples,10,sizeof(samples[0]), sort_ascend);

	
	int remove = (n_samples-good_samples)/2;
	int result = 0;

	for(uint8_t i = remove ; i < n_samples-remove; i++ )
	{
		result += samples[i];
		
	}

	result = (result/(n_samples-2*remove));
	return result;

}

int sort_ascend(const void *cmp1, const void *cmp2)
{

/*   int a = *((uint8_t *)cmp1);
  int b = *((uint8_t *)cmp2);
  // The comparison
  return (a-b); */
	// The comparison
	return (*((int16_t *)cmp1) - *((int16_t *)cmp2));

}
