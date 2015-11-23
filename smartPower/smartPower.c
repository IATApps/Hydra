/*
 * smartPower.c
 *
 * Created: 9/28/2012 2:03:41 PM
 *  Author: Caleb
 */ 

#ifdef F_CPU
#undef F_CPU
#endif
#define F_CPU 10000000UL

#include "asf.h"

#include "MCP4451.h"
#include "USART.h"
#include "I2C.h"
#include "util.h"
#include "ADC.h"
#include "config.h"
#include "packet_handler.h"

// Buffer for copying strings around
char g_string_buffer[10];

char* g_command_done_string = "\r\n  OK\r\n>";
char* g_command_failed_string = "\r\n  FAILED!\r\n>";
char* g_bad_supply_string = "\r\n  Invalid Supply\r\n>";
char* g_unknown_command_string = "\r\n  Bad Command\r\n>";

__attribute__((optimize("Os")))
// Main program entry point
int main(void)
{
	g_data.new_iout1 = 0;
	g_data.new_iout2 = 0;
	g_data.new_iout3 = 0;
	g_data.new_vin = 0;
	g_data.new_vout1 = 0;
	g_data.new_vout2 = 0;
	g_data.new_vout3 = 0;
	
	// Flags indicating whether the supplies are in CC mode
	g_data.out1_cc = 0;
	g_data.out2_cc = 0;
	g_data.out3_cc = 0;
	
	// Flags indicating output fault on each supply
	g_data.out1_fault = 0;
	g_data.out2_fault = 0;
	g_data.out3_fault = 0;
	
	// Flag indicating that the display is being updated
	g_refreshing_display = 0;
	
	// Flag indicating that new data should be transmitted
	g_send_data = 0;
	
	// Set supply status strings for display purposes
	g_supply1_status = g_status_off_string;
	g_supply2_status = g_status_off_string;
	g_supply3_status = g_status_off_string;
	
	// Input voltage cutoff not active
	g_data.voltage_cutoff_active = 0;
	
	// Wait for a while before starting up (allow supply to settle)
	_delay_ms(1000);
	
	i2c_init();
	usart_init();
	
	// Enable global interrupts
	SREG = 0x80;
	
	// Setup RUN 1, RUN 2, and RUN 3 as output pins
	RUN1_DDR |= RUN1_MASK;
	RUN2_DDR |= RUN2_MASK;
	RUN3_DDR |= RUN3_MASK;
	
	// Setup SYNC/PWM pins as output pins
	SYNC1_DDR |= SYNC1_MASK;
	SYNC2_DDR |= SYNC2_MASK;
	SYNC3_DDR |= SYNC3_MASK;
	
	// Setup voltage MUX address pins as outputs
	MUX_ADDR1_DDR |= MUX_ADDR1_MASK;
	MUX_ADDR2_DDR |= MUX_ADDR2_MASK;
		
	// Initialize and start the ADC
	init_adc();
	
	// Load settings from the EEPROM
	load_global_settings();
		
	// Set up the power supply outputs based on the current configuration
	update_outputs_from_config();
	
	// Start the timer to initiate data transmission
	start_timer1();
	
	// Main iteration loop	
    while(1)
    {
		// If we've received new ADC data, do stuff with it...
		// Output currents
		
		if( g_data.new_iout1 )
		{
			// Check to make sure that we aren't over the maximum current limit
			if( g_config.V1_enabled && (g_data.iout1 > g_config.C1_limit) )
			{
				// We have exceeded the maximum current limit.  Get a new target voltage to bring the current within specified limits.
				uint16_t new_vout = get_cc_target_voltage( g_data.vout1, g_data.iout1, g_config.C1_limit );
				g_config.POT1_taps = get_POT_taps(new_vout, 1);
				
				if( !update_output_voltage(1) )
				{
					g_data.out1_cc = 1;
					g_supply1_status = g_status_on_cc_string;
				}
			}
			// Now, if we are in current control mode, check to see if we are well below the target current.  If so, compute new voltage.
			// If new voltage is greater than the target output voltage, switch back to voltage control mode.
			else if( g_config.V1_enabled && g_data.out1_cc && (g_data.iout1 < (g_config.C1_limit - g_config.C1_limit/10)) )
			{
				// Compute a new vout
				uint16_t new_vout = get_cc_target_voltage( g_data.vout1, g_data.iout1, g_config.C1_limit );
				
				// If new voltage is greater than the target, switch back to voltage control mode
				if( new_vout >= g_config.V1_target )
				{
					g_data.out1_cc = 0;
					g_supply1_status = g_status_on_cv_string;
					g_config.POT1_taps = get_POT_taps(g_config.V1_target, 1);
										
					update_output_voltage(1);
				}
				// If the new voltage is not greater than the target, set new target voltage
				else
				{
					g_config.POT1_taps = get_POT_taps(new_vout, 1);
					update_output_voltage(1);
				}
			}
			
			copy_data_to_output();
			g_data.new_iout1 = 0;
		}
		
		if( g_data.new_iout2 )
		{
			// Check to make sure that we aren't over the maximum current limit
			if( g_config.V2_enabled && (g_data.iout2 > g_config.C2_limit) )
			{
				// We have exceeded the maximum current limit.  Get a new target voltage to bring the current within specified limits.
				uint16_t new_vout = get_cc_target_voltage( g_data.vout2, g_data.iout2, g_config.C2_limit );
				g_config.POT2_taps = get_POT_taps(new_vout, 2);
				
				if( !update_output_voltage(2) )
				{
					g_data.out2_cc = 1;
					g_supply2_status = g_status_on_cc_string;
				}				
			}
			// Now, if we are in current control mode, check to see if we are well below the target current.  If so, compute new voltage.
			// If new voltage is greater than the target output voltage, switch back to voltage control mode.
			else if( g_config.V2_enabled && g_data.out2_cc && (g_data.iout2 < (g_config.C2_limit - g_config.C2_limit/10)) )
			{
				// Compute a new vout
				uint16_t new_vout = get_cc_target_voltage( g_data.vout2, g_data.iout2, g_config.C2_limit );
				
				// If new voltage is greater than the target, switch back to voltage control mode
				if( new_vout >= g_config.V2_target )
				{
					g_data.out2_cc = 0;
					
					g_supply2_status = g_status_on_cv_string;
					
					g_config.POT2_taps = get_POT_taps(g_config.V2_target, 2);
					
					update_output_voltage(2);
				}
				// If the new voltage is not greater than the target, set new target voltage
				else
				{
					g_config.POT2_taps = get_POT_taps(new_vout, 2);
					update_output_voltage(2);
				}
			}
			
			copy_data_to_output();
			g_data.new_iout2 = 0;
		}
		if( g_data.new_iout3 )
		{
			// Check to make sure that we aren't over the maximum current limit
			if( g_config.V3_enabled && (g_data.iout3 > g_config.C3_limit) )
			{
				// We have exceeded the maximum current limit.  Get a new target voltage to bring the current within specified limits.
				uint16_t new_vout = get_cc_target_voltage( g_data.vout3, g_data.iout3, g_config.C3_limit );
				g_config.POT3_taps = get_POT_taps(new_vout, 3);
				
				if( !update_output_voltage(3) )
				{
					g_data.out3_cc = 1;
					g_supply3_status = g_status_on_cc_string;
				}
			}
			// Now, if we are in current control mode, check to see if we are well below the target current.  If so, compute new voltage.
			// If new voltage is greater than the target output voltage, switch back to voltage control mode.
			else if( g_config.V3_enabled && g_data.out3_cc && (g_data.iout3 < (g_config.C3_limit - g_config.C3_limit/10)) )
			{
				// Compute a new vout
				uint16_t new_vout = get_cc_target_voltage( g_data.vout3, g_data.iout3, g_config.C3_limit );
				
				// If new voltage is greater than the target, switch back to voltage control mode
				if( new_vout >= g_config.V3_target )
				{
					g_data.out3_cc = 0;
					
					g_supply3_status = g_status_on_cv_string;
					
					g_config.POT3_taps = get_POT_taps(g_config.V3_target, 3);
					
					update_output_voltage(3);
				}
				// If the new voltage is not greater than the target, set new target voltage
				else
				{
					g_config.POT3_taps = get_POT_taps(new_vout, 3);
					update_output_voltage(3);
				}
			}			
			
			copy_data_to_output();
			g_data.new_iout3 = 0;
		}
		
		// Output voltages
		if( g_data.new_vout1 )
		{
			copy_data_to_output();
			g_data.new_vout1 = 0;
		}
		if( g_data.new_vout2 )
		{
			copy_data_to_output();
			g_data.new_vout2 = 0;
		}
		if( g_data.new_vout3 )
		{
			copy_data_to_output();
			g_data.new_vout3 = 0;
		}
		
		// Input voltage
		if( g_data.new_vin )
		{
			// Check to make sure that the input voltage isn't lower than the cutoff threshold.  If it is, disable the outputs and go to sleep.
			// Once voltage cutoff has been enabled, always make sure that the supplies are turned off unless the cutoff is overriden.  This ensures
			// that once the cutoff occurs, it stays cut off until the power is cycled.  We do this to avoid a high-current load being turned on and
			// off repeatedly.
			if( (g_data.vin < g_config.Vin_cutoff) || g_data.voltage_cutoff_active )
			{
				g_data.voltage_cutoff_active = 1;
				
				if( !g_config.V1_cutoff_disabled )
				{
					disable_output_temp(1);
				}
				
				if( !g_config.V2_cutoff_disabled )
				{
					disable_output_temp(2);
				}
				
				if( !g_config.V3_cutoff_disabled )
				{
					disable_output_temp(3);
				}
			}
			
			copy_data_to_output();
			g_data.new_vin = 0;
		}
		
		// If the g_send_data flag is set, then new data needs to be transmitted over the UART
		// Send data based on whether we are in binary mode or ascii mode
		if( g_send_data && (g_binary_mode == 0) )
		{
			g_send_data = 0;
		}
		else if( g_send_data && (g_binary_mode == 1) )	// Must check g_binary_mode again in case it changed since the last conditional check (it is volatile)
		{
			g_send_data = 0;

			send_global_data(V1_STATUS, ADDRESS_TYPE_DATA, PACKET_IS_BATCH, BATCH_SIZE_4);

		}
		
		// If a packet was received, handle packet.
		// Packet size of 1 indicates that a carriage return was written with no additional
		// info.  Erase the screen and draw the display.
		if( g_new_packet && (g_packet_size == 1) )
		{
			g_new_packet = 0;
			draw_display();
		}
		if( g_new_packet && (g_packet_size > 1) )		// Need to check g_packet_size again because it can be changed by an interrupt routine after the previous conditional executes!
		{			
			g_new_packet = 0;
			
			if( g_binary_mode == 0 )
			{
				// Tokenize the packet
				char* token = strtok((char*)g_packet_buf, " \r");
				
				// Check for conversion to binary COM mode
				if( !strcmp(":b", token) )
				{
					g_binary_mode = 1;
					USART_transmit("\r\nEntering binary COM mode.\r\n");
				}
				// Set power supply 1 output voltage
				else if( !strcmp("set1", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					g_config.V1_target = ConvertAtoMV(token);
					
					// Convert desired voltage to POT taps
					g_config.POT1_taps = get_POT_taps(g_config.V1_target, 1);
					
					save_config();
					
					if( update_output_voltage(1) )
					{
						USART_transmit(g_command_failed_string);
					}
					else
					{
						USART_transmit(g_command_done_string);
					}
					
					// Restart timer for display purposes
					start_timer1();
				}
				// Set power supply 2 output voltage
				else if( !strcmp("set2", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					g_config.V2_target = ConvertAtoMV(token);
					
					// Convert desired voltage to POT taps
					g_config.POT2_taps = get_POT_taps(g_config.V2_target, 2);
					
					save_config();
					
					if( update_output_voltage(2) )
					{
						USART_transmit(g_command_failed_string);
					}
					else
					{
						USART_transmit(g_command_done_string);
					}
					
					// Restart timer for display purposes
					start_timer1();
				}
				// Set power supply 3 output voltage
				else if( !strcmp("set3", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					g_config.V3_target = ConvertAtoMV(token);
					
					// Convert desired voltage to POT taps
					g_config.POT3_taps = get_POT_taps(g_config.V3_target, 3);
					
					save_config();
					
					if( update_output_voltage(3) )
					{
						USART_transmit(g_command_failed_string);
					}
					else
					{
						USART_transmit(g_command_done_string);
					}
					
					// Restart timer for display purposes
					start_timer1();
				}
				// Set power supply 1 output current limit
				else if( !strcmp("setc1", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					// We can use the ConvertAtoMV function to convert the float input to mA
					// The function call doesn't actually care about units
					g_config.C1_limit = ConvertAtoMV(token);
					
					save_config();
					
					USART_transmit(g_command_done_string);
				}
				// Set power supply 1 output current limit
				else if( !strcmp("setc2", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					// We can use the ConvertAtoMV function to convert the float input to mA
					// The function call doesn't actually care about units
					g_config.C2_limit = ConvertAtoMV(token);
					
					save_config();
					
					USART_transmit(g_command_done_string);
				}
				// Set power supply 1 output current limit
				else if( !strcmp("setc3", token) )
				{
					// Get next token
					token = strtok(NULL, " \r");
					// We can use the ConvertAtoMV function to convert the float input to mA
					// The function call doesn't actually care about units
					g_config.C3_limit = ConvertAtoMV(token);
					
					save_config();
					
					USART_transmit(g_command_done_string);
				}
				// Enable power supply output
				else if( !strcmp("en",token) )
				{
					token = strtok(NULL, " \r");
					int supply = ConvertAtoI(token);
					
					if( supply > 0 && supply < 4 )
					{
						enable_output(supply);
						save_config();
						USART_transmit(g_command_done_string);
					}
					else
					{
						USART_transmit(g_bad_supply_string);
					}
					
					// Restart timer for display purposes
					start_timer1();
				}
				// Disable power supply output
				else if( !strcmp("dis",token) )
				{
					token = strtok(NULL, " \r");
					int supply = ConvertAtoI(token);
					
					if( supply > 0 && supply < 4 )
					{
						disable_output(supply);
						save_config();
						USART_transmit(g_command_done_string);
					}
					else
					{
						USART_transmit(g_bad_supply_string);
					}
					
					// Restart timer for display purposes
					start_timer1();
				}
				// Unknown command
				else
				{
					USART_transmit_block(g_unknown_command_string);
					
					// Restart timer for display purposes
					start_timer1();
				}
				
			}		// END "if not in binary mode"
			// We are in binary COM mode.  
			else
			{
				// We received a raw packet.  Copy it to a local variable and process it
				Packet new_packet = g_received_packet;
				
				process_packet( &new_packet );				
			}
		}	// End of "if packet found"
		
    }	// End of main loop
	
}