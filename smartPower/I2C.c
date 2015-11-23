/*
 * I2C.c
 *
 * Created: 12/6/2012 4:40:35 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "I2C.h"


// Initialize i2c bus
void i2c_init()
{
	// Set SCL rate
	TWBR = 32;	// (100 kHz with a 8Mhz system clock)
	
	// Enable i2c bus
	TWCR = (1 << TWEN) | (1 << TWINT);
}


// Writes a data byte to the i2c bus.  Start condition and address must be set already.
char i2c_data_write( uint8_t data )
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while( !(TWCR & (1 << TWINT)) );
	if( (TWSR & 0xF8) != MT_DATA_ACK )
	{
		//		USART_transmit("No ack from POT on data write\r");
		TWCR = (1 << TWINT) | ( 1 << TWEN) | (1 << TWSTO);
		return 1;
	}
	
	return 0;
}
