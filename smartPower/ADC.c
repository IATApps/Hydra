/*
 * ADC.c
 *
 * Created: 2/1/2013 6:03:44 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "ADC.h"
#include "util.h"
#include "config.h"
#include "USART.h"
#include "packet_handler.h"

#define STATE_ADC_IDLE		0
#define STATE_ADC_VIN		1
#define STATE_ADC_VOUT1		2
#define STATE_ADC_VOUT2		3
#define STATE_ADC_VOUT3		4
#define STATE_ADC_IOUT1		5
#define STATE_ADC_IOUT2		6
#define STATE_ADC_IOUT3		7

volatile uint8_t g_ADC_state;
volatile uint8_t g_channel;

volatile uint8_t g_send_data;

// Timer interrupt handling routine
ISR(TIMER1_COMPA_vect)
{	
	TCCR1B = 0;		// Stop timer
	g_send_data = 1;
	
	// Restart timer
	start_timer1();
}

// Interrupt handler for completed ADC conversion
// This relies on global variables defined in data.h and data.c
ISR(ADC_vect)
{
	// Read in ADC conversion value to its proper location and set next conversion settings
	switch(g_ADC_state)
	{
		case STATE_ADC_VOUT1:
			g_data.vout1_sum += get_conversion_value();
			
			g_data.vout1_sample_count++;
			if( g_data.vout1_sample_count == AVG_SIZE )
			{
				g_data.vout1_raw = (g_data.vout1_sum/AVG_SIZE)*VOUT_RAW_TO_V_SCALE;
				g_data.vout1 = g_data.vout1_raw/10;
				
				g_data.vout1_sample_count = 0;
				g_data.vout1_sum = 0;
				
				g_data.new_vout1 = 1;
			}
			
			g_ADC_state = STATE_ADC_IOUT1;
			
			g_channel = IOUT1_ADC_CHANNEL;
			
			// Change the MUX address for the next VOUT read (needs time to settle)
			set_mux_address(VOUT2_MUX_CHANNEL);
		break;
		
		case STATE_ADC_IOUT1:
			g_data.iout1_sum += get_conversion_value();
			
			g_data.iout1_sample_count++;
			if( g_data.iout1_sample_count == AVG_SIZE )
			{
				g_data.iout1_raw = (g_data.iout1_sum/AVG_SIZE)*IOUT_RAW_TO_I_SCALE;
				g_data.iout1 = g_data.iout1_raw/10;
				
				g_data.iout1_sample_count = 0;
				g_data.iout1_sum = 0;
				
				g_data.new_iout1 = 1;
			}
			g_data.new_iout1 = 1;
			
			g_ADC_state = STATE_ADC_VOUT2;
			g_channel = VOUT_CHANNEL;
		break;
		
		case STATE_ADC_VOUT2:
			g_data.vout2_sum += get_conversion_value();
			
			g_data.vout2_sample_count++;
			if( g_data.vout2_sample_count == AVG_SIZE )
			{
				g_data.vout2_raw = (g_data.vout2_sum/AVG_SIZE)*VOUT_RAW_TO_V_SCALE;
				g_data.vout2 = g_data.vout2_raw/10;
				
				g_data.vout2_sample_count = 0;
				g_data.vout2_sum = 0;
				
				g_data.new_vout2 = 1;
			}
			
			g_ADC_state = STATE_ADC_IOUT2;
			
			g_channel = IOUT2_ADC_CHANNEL;
			
			// Change the MUX address for the next VOUT read (needs time to settle)
			set_mux_address(VOUT3_MUX_CHANNEL);
		break;
		
		case STATE_ADC_IOUT2:
			g_data.iout2_sum += get_conversion_value();
			
			g_data.iout2_sample_count++;
			if( g_data.iout2_sample_count == AVG_SIZE )
			{
				g_data.iout2_raw = (g_data.iout2_sum/AVG_SIZE)*IOUT_RAW_TO_I_SCALE;
				g_data.iout2 = g_data.iout2_raw/10;
				
				g_data.iout2_sample_count = 0;
				g_data.iout2_sum = 0;
				
				g_data.new_iout2 = 1;
			}
			
			g_ADC_state = STATE_ADC_VOUT3;
			g_channel = VOUT_CHANNEL;
		break;
		
		case STATE_ADC_VOUT3:
			g_data.vout3_sum += get_conversion_value();
			
			g_data.vout3_sample_count++;
			if( g_data.vout3_sample_count == AVG_SIZE )
			{
				g_data.vout3_raw = (g_data.vout3_sum/AVG_SIZE)*VOUT_RAW_TO_V_SCALE;
				g_data.vout3 = g_data.vout3_raw/10;
				
								
				g_data.vout3_sample_count = 0;
				g_data.vout3_sum = 0;
				
				g_data.new_vout3 = 1;
			}
			
			g_ADC_state = STATE_ADC_IOUT3;
			g_channel = IOUT3_ADC_CHANNEL;
			
			// Change the MUX address for the next VOUT read (needs time to settle)
			set_mux_address(VOUT1_MUX_CHANNEL);
		break;
		
		case STATE_ADC_IOUT3:
			g_data.iout3_sum += get_conversion_value();
			
			g_data.iout3_sample_count++;
			if( g_data.iout3_sample_count == AVG_SIZE )
			{
				g_data.iout3_raw = (g_data.iout3_sum/AVG_SIZE)*IOUT_RAW_TO_I_SCALE;
				g_data.iout3 = g_data.iout3_raw/10;
				
				g_data.iout3_sample_count = 0;
				g_data.iout3_sum = 0;
				
				g_data.new_iout3 = 1;
			}
			
			g_ADC_state	= STATE_ADC_VIN;
			g_channel = VIN_CHANNEL;
		break;

		case STATE_ADC_VIN:
			g_data.vin_sum += get_conversion_value();
			
			g_data.vin_sample_count++;
			if( g_data.vin_sample_count == AVG_SIZE)
			{
				g_data.vin_raw = (g_data.vin_sum/AVG_SIZE)*VOUT_RAW_TO_V_SCALE;
				g_data.vin = g_data.vin_raw/10;
				
				g_data.vin_sample_count = 0;
				g_data.vin_sum = 0;
				
				g_data.new_vin = 1;
			}
			
			g_ADC_state = STATE_ADC_VOUT1;
			g_channel = VOUT_CHANNEL;
		break;
		
		default:
			g_ADC_state = STATE_ADC_IOUT3;
			g_channel = IOUT3_ADC_CHANNEL;
			
			set_mux_address(VOUT1_MUX_CHANNEL);
		break;
	}
	
	// Start timer again to trigger next conversion after input multiplexer has settled
//	start_timer1();

	start_conversion(g_channel);
}

// Initialize the ADC
void init_adc()
{
	g_ADC_state = STATE_ADC_VOUT1;
	g_channel = VOUT_CHANNEL;
	set_mux_address(VOUT1_MUX_CHANNEL);
	
	g_data.vin_sample_count = 0;
	g_data.vout1_sample_count = 0;
	g_data.vout2_sample_count = 0;
	g_data.vout3_sample_count = 0;
	
	g_data.iout1_sample_count = 0;
	g_data.iout2_sample_count = 0;
	g_data.iout3_sample_count = 0;
	
	// Disable the ADC
	ADCSRA = 0;
	
	// Set the prescaler for the ADC clock
//	ADCSRA |= (5 << ADPS0);		// 8e6 / 64 = 125e3
	ADCSRA |= (5 << ADPS0);		// 10e6 / 64 = 156e3
	
	// Enable the ADC interrupt
	ADCSRA |= (1 << ADIE);
	
	// Disable digital input buffers on select ADC input pins (channels 0, 1, and 2 for current measurement)
	DIDR0 = 7;		// That's 0b0000111 to disable channels 0, 1, and 2
	
	// Enable the ADC (doesn't start a conversion.  That needs to happen later.)
	ADCSRA |= (1 << ADEN);
	
	start_conversion(g_channel);
}

// Function call for starting the timer
void start_timer1()
{
	// Initialize timer1 to fire once every couple milliseconds to trigger a new ADC conversion
	TCCR1B = 0;			// Stop timer if it is running already
	TCNT1 = 0;			// Clear timer
	TIMSK1 = 0x02;		// Enable interrupt on output capture 1
	OCR1A = 3906;			// Set counter overflow value for first channel (At 7812.5 Hz, 3906 is about 500 milliseconds)
	TCCR1B = 0x01 | (5 << CS10);		// Enable timer, 1024 prescaler (10e6/1024 = 7812.5 Hz), clear on compare match
}

// Sample the specified channel.  This is a blocking call
uint16_t sample_channel( uint8_t channel )
{
	uint8_t interrupt_settings;
	uint16_t data;
	
	// Store interrupt settings
	interrupt_settings = ADCSRA;
	
	// Make sure interrupts are disabled
	ADCSRA &= ~(1 << ADIE);
	
	start_conversion(channel);
	
	// Wait for conversion to complete
	while( (ADCSRA & (1 << ADIF)) == 0 );
	data = get_conversion_value();
	
	// Clear the interrupt flag
	ADCSRA |= (1 << ADIF);
	
	// Restore interrupt settings
	ADCSRA = interrupt_settings;
	
	// Done!  Return the converted value
	return data;
}

// Start an ADC conversion, but return immediately (non-blocking)
void start_conversion( uint8_t channel )
{
	// Clear the currently selected channel
	ADMUX &= ~(0x0F);
	
	// Select the specified ADC channel
	ADMUX |= (channel & 0x0F);
		
	// Start the conversion
	ADCSRA |= (1 << ADSC);
	
	// Done!
}

// Get the converted ADC value from the ADC registers
uint16_t get_conversion_value( void )
{
	return (uint16_t)(ADCL | ((uint16_t)ADCH << 8));
}