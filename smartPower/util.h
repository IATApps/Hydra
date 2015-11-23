/*
 * util.h
 *
 * Created: 12/6/2012 4:39:19 PM
 *  Author: Caleb
 */ 


#ifndef UTIL_H_
#define UTIL_H_

#include "asf.h"

// Pins to control output voltage MUX
#define MUX_ADDR1_DDR			DDRD
#define MUX_ADDR1_MASK			(1 << DDD4)
#define MUX_ADDR1_PORT			PORTD

#define MUX_ADDR2_DDR			DDRD
#define MUX_ADDR2_MASK			(1 << DDD3)
#define MUX_ADDR2_PORT			PORTD

#define EXTERNAL_MUX_SWITCH_DELAY	200

// Definitions for the ASCII-mode display table
#define SUPPLY1_ROW				2
#define SUPPLY2_ROW				3
#define SUPPLY3_ROW				4
#define STATUS_ROW				5
#define STATUS_COLUMN			5

#define SUPPLY_STATUS_COLUMN	37
#define SUPPLY_VOLTAGE_COLUMN	13
#define SUPPLY_CURRENT_COLUMN	24

// Delay in milliseconds used to make sure VT100 commands perform correctly
#define VT100_DISPLAY_DELAY		2

int ConvertAtoI( char* str );
int ConvertItoA(int32_t integer, char* str_buf, int buffer_length);
int ConvertAtoMV( char* str );
int CHR_isdigit( char c );
void get_voltage_string( int32_t voltage_tmv, char* output_buf );

uint16_t get_cc_target_voltage( uint16_t voltage_mv, uint16_t current_ma, uint16_t max_current_ma );

// Functions for vt100 terminal interface
void vt100_erase_screen( void );
void vt100_cursor_location(uint8_t row, uint8_t col);
void refresh_display( void );
void write_status_output(char* status);

void draw_display( void );

void set_mux_address( uint8_t address );

uint16_t compute_avg( uint16_t* input_array, uint16_t elements );

#endif /* UTIL_H_ */
