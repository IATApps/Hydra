/*
 * MCP4451.h
 *
 * Created: 12/6/2012 4:26:03 PM
 *  Author: Caleb
 */ 

#include <avr/io.h>

#ifndef MCP4451_H_
#define MCP4451_H_

#define PWR1_SLAVE_ADDRESS		0x2C
#define PWR2_SLAVE_ADDRESS		0x2D
#define PWR3_SLAVE_ADDRESS		0x2E

#define RUN1_DDR				DDRD
#define RUN1_MASK				(1 << DDD5)
#define RUN1_PORT				PORTD

#define RUN2_DDR				DDRB
#define RUN2_MASK				(1 << DDB0)
#define RUN2_PORT				PORTB

#define RUN3_DDR				DDRD
#define RUN3_MASK				(1 << DDD2)
#define RUN3_PORT				PORTD

#define SYNC1_DDR				DDRD
#define SYNC1_MASK				(1 << DDD6)
#define SYNC1_PORT				PORTD

#define SYNC2_DDR				DDRB
#define SYNC2_MASK				(1 << DDB1)
#define SYNC2_PORT				PORTB

#define SYNC3_DDR				DDRD
#define SYNC3_MASK				(1 << DDD7)
#define SYNC3_PORT				PORTD

#define I2C_TIMEOUT				500000


int set_digital_pot( uint8_t slave_address, int32_t new_voltage );
char POT_set_wiper(uint8_t wiper_number, uint16_t data);

#endif /* MCP4451_H_ */