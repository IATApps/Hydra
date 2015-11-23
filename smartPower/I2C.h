/*
 * I2c.h
 *
 * Created: 12/6/2012 4:40:51 PM
 *  Author: Caleb
 */ 


#ifndef I2C_H_
#define I2C_H_

// Some i2c definitions
#define START			0x08
#define MT_SLA_ACK		0x18
#define MT_DATA_ACK		0x28


void i2c_init( void );
char i2c_data_write( uint8_t data );


#endif /* I2C_H_ */