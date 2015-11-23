/*
 * ADC.h
 *
 * Created: 2/1/2013 6:04:05 PM
 *  Author: Caleb
 */ 


#ifndef ADC_H_
#define ADC_H_

#include "asf.h"

void init_adc( void );
uint16_t sample_channel( uint8_t channel );
void start_conversion( uint8_t channel );
uint16_t get_conversion_value( void );

void start_timer1( void );

extern volatile uint8_t g_send_data;

// ADC pins
#define IOUT1_DDR				DDRC
#define IOUT1_MASK				(1 << DDC1)
#define IOUT1_PORT				PORTC

#define IOUT2_DDR				DDRC
#define IOUT2_MASK				(1 << DDC0)
#define IOUT2_PORT				PORTC

#define IOUT3_DDR				DDRC
#define IOUT3_MASK				(1 << DDC2)
#define IOUT3_PORT				PORTC

// ADC channels
#define IOUT1_ADC_CHANNEL		1
#define IOUT2_ADC_CHANNEL		0
#define IOUT3_ADC_CHANNEL		2
#define VOUT_CHANNEL			6
#define VIN_CHANNEL				7
#define TEMPERATURE_CHANNEL		8

// External MUX addresses
#define VOUT1_MUX_CHANNEL		1
#define VOUT2_MUX_CHANNEL		2
#define VOUT3_MUX_CHANNEL		3

#endif /* ADC_H_ */
