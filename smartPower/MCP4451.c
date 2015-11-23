/*
 * MCP4451.c
 *
 * Created: 12/6/2012 4:25:37 PM
 *  Author: Caleb
 */ 

#include <avr/io.h>
#include "MCP4451.h"
#include "I2C.h"

// Function for setting the output voltage
int set_digital_pot( uint8_t address, int32_t POT_taps )
{
	
	uint32_t timeout;
	
	if( POT_taps < 0 )
	{
		POT_taps = 0;
	}
	
	if( POT_taps > 1024 )
	{
		POT_taps = 1024;
	}
	
	// Figure out the values of the individual registers on the POT
//	uint16_t high_order_reg = POT_taps % 257;
	uint16_t low_order_count = (uint8_t)(POT_taps/257);
	
	uint16_t wiper1_data = 0;
	uint16_t wiper2_data = 0;
	uint16_t wiper3_data = 0;
	uint16_t wiper4_data = 0;
	
	if( low_order_count > 3 )
	{
		//		USART_transmit("Invalid setting\r");
		return 1;
	}
	
	// Set wiper4 value
	if( POT_taps >= 257 )
	{
		wiper4_data = 257;
	}
	else
	{
		wiper4_data = POT_taps;
	}
	POT_taps -= 257;
	
	// Set wiper3 value
	if( POT_taps >= 257 )
	{
		wiper3_data = 257;
	}
	else if( POT_taps > 0 )
	{
		wiper3_data = POT_taps;
	}
	POT_taps -= 257;
	
	// Set wiper2 value
	if( POT_taps >= 257 )
	{
		wiper2_data = 257;
	}
	else if( POT_taps > 0 )
	{
		wiper2_data = POT_taps;
	}
	POT_taps -= 257;
	
	// Set wiper1 value
	if( POT_taps >= 257 )
	{
		wiper1_data = 257;
	}
	else if( POT_taps > 0 )
	{
		wiper1_data = POT_taps;
	}
	
	// Initiate start condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	// Wait until start condition finishes transmitting
	timeout = 0;
	while( !(TWCR & (1 << TWINT)) )	
	{
		if( timeout++ > I2C_TIMEOUT )
		{
			return 1;
		}
	}
	if( (TWSR & 0xF8) != START )
	{
		//		USART_transmit("i2c start failed\r");
		return 2;
	}
	
	// Slave address
	TWDR = (address << 1);					// Address
	TWCR = (1 << TWEN) | (1 << TWINT);		// Clear interrupt to trigger address write
	timeout = 0;
	while( !(TWCR & (1 << TWINT)) )
	{
		if( timeout++ > I2C_TIMEOUT )
		{
			return 1;
		}
	}
	if( (TWSR & 0xF8) != MT_SLA_ACK )
	{
		//		USART_transmit("no ack from POT\r");
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 3;
	}
	
	// Set first wiper
	if( POT_set_wiper(1, wiper1_data) )
	{
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 4;
	}
	
	// Set second wiper
	if( POT_set_wiper(2, wiper2_data) )
	{
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 5;
	}
	
	// Set third wiper
	if( POT_set_wiper(3, wiper3_data) )
	{
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 6;
	}
	
	// Set fourth wiper
	if( POT_set_wiper(4, wiper4_data) )
	{
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 7;
	}
	
	// Finished! Send stop condition
	TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
	
	return 0;
}

// Sets a POT wiper value.  wiper_number must be one of {1,2,3,4}.  data should be between 0 and 257.
// The i2c start condition and address should have been sent already.
char POT_set_wiper(uint8_t wiper_number, uint16_t data)
{
	uint8_t cmd_address;
	
	switch(wiper_number)
	{
		case 1:
			cmd_address = 0x00;
			break;
		case 2:
			cmd_address = 0x10;
			break;
		case 3:
			cmd_address = 0x60;
			break;
		case 4:
			cmd_address = 0x70;
			break;
		default:
			return 1;
	}
	
	// Get highest-order data bit
	cmd_address |= ((data >> 8) & 0x01);
	
	if( i2c_data_write(cmd_address) )
	{
		return 1;
	}
	
	if( i2c_data_write(data & 0x0FF) )
	{
		return 1;
	}
	
	return 0;
}
