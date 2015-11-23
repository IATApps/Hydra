/*
 * util.c
 *
 * Created: 12/6/2012 4:39:10 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "util.h"
#include "USART.h"
#include "config.h"

char vt100_buf[10];

char g_refreshing_display;
char* g_newline_string = "\r\n";


// Set the analog MUX address
void set_mux_address( uint8_t address )
{
	switch(address)
	{
		case 1:
		MUX_ADDR1_PORT |= MUX_ADDR1_MASK;
		MUX_ADDR2_PORT &= ~MUX_ADDR2_MASK;
		break;
		
		case 2:
		MUX_ADDR1_PORT &= ~MUX_ADDR1_MASK;
		MUX_ADDR2_PORT |= MUX_ADDR2_MASK;
		break;
		
		case 3:
		MUX_ADDR1_PORT |= MUX_ADDR1_MASK;
		MUX_ADDR2_PORT |= MUX_ADDR2_MASK;
		break;
		
		default:
		MUX_ADDR1_PORT &= ~MUX_ADDR1_MASK;
		MUX_ADDR2_PORT &= ~MUX_ADDR2_MASK;
		break;
	}
}


/*******************************************************************************
* Function Name  : int ConvertAtoI( char* str )
* Input          :
* Output         :
* Return         : The integer equivalent of the ASCII number representation
* Description    :

*******************************************************************************/
int ConvertAtoI( char* str )
{
	int output = 0;
	int negative = 0;
	
	if( *str == '-' )
	{
		negative = 1;
		str++;
	}

	while( CHR_isdigit(*str) )
	{
		output = output*10 + (*str++ - '0');
	}

	if( negative )
	{
		return -output;
	}

	return output;
}

// Function for converting integer to string.  Will not overwrite the buffer
// if the string length works out to be larger than the buffer length.  Returns 1 if
// failed due to inadequate buffer length, 0 otherwise
int ConvertItoA(int32_t integer, char* str_buf, int buffer_length)
{
	int32_t temp;
	uint8_t negative;
	uint16_t digits;
	uint32_t index;
	
	// See if this is a negative quantity
	digits = 0;
	negative = 0;
	if( integer < 0 )
	{
		integer = -integer;
		negative = 1;
		digits++;
	}
	
	// Determine the number of digits in the string
	temp = integer;
	do
	{
		digits++;
		temp = temp/10;
	} while( temp > 0 );
	
	// Make sure the buffer length is large enough to hold all the digits plus the null
	// terminating character
	if( digits > (uint16_t)buffer_length )
	{
		return 1;
	}
	
	// Produce string
	index = digits - 1;
	do
	{
		str_buf[index--] = '0' + (integer % 10);
		integer = integer/10;
	} while( integer > 0 );
	
	if( negative )
	{
		str_buf[0] = '-';
	}
	
	str_buf[digits] = '\0';
	
	return 0;
}

/*******************************************************************************
* Function Name  : uint32_t get_cc_target_voltage( uint16_t voltage_mv, uint16_t current_ma, uint16_t max_current_ma )
* Input          :
* Output         :
* Return         : A new target voltage in millivolts
* Description    :

Receives a voltage in mv and a current in mv, computes a load resistance,
and estimates a target voltage to bring the current equal to the maximum allowed current

*******************************************************************************/
uint16_t get_cc_target_voltage( uint16_t voltage_mv, uint16_t current_ma, uint16_t max_current_ma )
{
	// Compute the load resistance and set a new output voltage based on it.
	// Multiplies by a scale factor of 1000 to get resistance in milliohms (so we don't have to do floating point operations)
	uint32_t resistance = ((uint32_t)voltage_mv)*1000/current_ma;
	// Compute new voltage target in millivolts.  The current limit is in milliamps and the resistance is in milliohms, so the
	// new target voltage is thousandths of millivolts.  Divide by 1000 to get millivolts.
	uint16_t new_vout = (uint16_t)(((uint32_t)max_current_ma*resistance)/1000);
	
	return new_vout;
}

/*******************************************************************************
* Function Name  : int ConvertAtoMV( char* str )
* Input          :
* Output         :
* Return         : The integer equivalent of the ASCII number representation
* Description    :

This function takes a floating-point input and effectively multiplies it by 1000
and converts to an integer.  The motive is to convert an input voltage to millivolts.

*******************************************************************************/
int ConvertAtoMV( char* str )
{
	int output = 0;
	int negative = 0;
	int decimal_digits = 0;
	int decimal = 0;
	
	if( *str == '-' )
	{
		negative = 1;
		str++;
	}

	while( CHR_isdigit(*str) || (*str == '.'))
	{
		if ( decimal && (*str != '.'))
		{
			decimal_digits++;
		}
		
		if (*str != '.')
		{
			output = output*10 + (*str - '0');
		}
		else
		{
			decimal = 1;
		}
		
		str++;
	}
	
	// Convert to millivolts
	while( decimal_digits > 3 )
	{
		output = output/10;
		decimal_digits--;
	}
	
	while( decimal_digits < 3 )
	{
		output = output*10;
		decimal_digits++;
	}

	// Add negative if applicable
	if( negative )
	{
		return -output;
	}

	return output;
}

/*******************************************************************************
* Function Name  : int CHR_isdigit( char c )
* Input          :
* Output         :
* Return         : 1 if digit, 0 if not
* Description    :

*******************************************************************************/
int CHR_isdigit( char c )
{
	if ((c>='0') && (c<='9'))
	{
		return 1;
	}
	
	return 0;
}

/*******************************************************************************
* Function Name  : int32_t compute_avg
* Input          :
* Output         :
* Return         : the computed average
* Description    :

Computes the average value of the elements in input_array with 'elements' length

*******************************************************************************/
uint16_t compute_avg( uint16_t* input_array, uint16_t elements )
{
	int32_t index;
	
	uint32_t avg = 0;
	for( index = 0; index < elements; index++ )
	{
		avg += input_array[index];
	}
	avg = avg/elements;
	
	return (uint16_t)avg;
}

// Function call for erasing a VT100 terminal display
void vt100_erase_screen()
{
	char buf[] = {0x1B,'[','2','J','\0'};
		
	USART_transmit_block(buf);
}


// Function call for setting the position of the cursor
// This command is of the form <ESC>[{ROW};{COLUMN}H
void vt100_cursor_location(uint8_t row, uint8_t col)
{	
	vt100_buf[0] = 0x1B;
	vt100_buf[1] = '[';
	vt100_buf[2] = '\0';
	
	USART_transmit_block(vt100_buf);
	
	ConvertItoA((int)row,vt100_buf,10);
	
	USART_transmit_block(vt100_buf);
	
	vt100_buf[0] = ';';
	vt100_buf[1] = '\0';
	
	USART_transmit_block(vt100_buf);
	
	ConvertItoA((int)col,vt100_buf,10);
	
	USART_transmit_block(vt100_buf);
	
	vt100_buf[0] = 'H';
	vt100_buf[1] = '\0';
	
	USART_transmit_block(vt100_buf);	
}

// Function call for drawing power supply information to the terminal
__attribute__((optimize("Os")))
void draw_display()
{
	g_refreshing_display = 1;
	
	/*
	vt100_erase_screen();
	vt100_cursor_location(1,1);
	*/
	
	USART_transmit_block("\r\nVin: ");
	
	get_voltage_string((int32_t)g_data.vin*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	USART_transmit_block(g_newline_string);
	
	USART_transmit_block("\tVoltage\t   Current\t Status\r\n");
	
	// Output supply 1 data
	USART_transmit_block("Vout1\t");
	get_voltage_string((int32_t)g_data.vout1*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	get_voltage_string((int32_t)g_data.iout1*10, vt100_buf);
	USART_transmit_block("\t   ");
	USART_transmit_block(vt100_buf);
	USART_transmit_block("/");
	get_voltage_string((int32_t)g_config.C1_limit*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	USART_transmit_block("\t ");
	USART_transmit_block(g_supply1_status);
	
	USART_transmit_block(g_newline_string);
	
	// Output supply 2 data
	USART_transmit_block("Vout2\t");
	get_voltage_string((int32_t)g_data.vout2*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	get_voltage_string((int32_t)g_data.iout2*10, vt100_buf);
	USART_transmit_block("\t   ");
	USART_transmit_block(vt100_buf);
	USART_transmit_block("/");
	get_voltage_string((int32_t)g_config.C2_limit*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	USART_transmit_block("\t ");
	USART_transmit_block(g_supply2_status);
	
	USART_transmit_block(g_newline_string);
	
	// Output supply 3 data
	USART_transmit_block("Vout3\t");
	get_voltage_string((int32_t)g_data.vout3*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	get_voltage_string((int32_t)g_data.iout3*10, vt100_buf);
	USART_transmit_block("\t   ");
	USART_transmit_block(vt100_buf);
	USART_transmit_block("/");
	get_voltage_string((int32_t)g_config.C3_limit*10, vt100_buf);
	USART_transmit_block(vt100_buf);
	
	USART_transmit_block("\t ");
	USART_transmit_block(g_supply3_status);
	
	USART_transmit_block("\r\n\r\n>");
	
	// Write anything to the screen that the user has already typed
	USART_transmit_length(g_RX_buf,g_RX_size);
	
	g_refreshing_display = 0;
}

// Function call for getting a string for the voltage specified in voltage_tmv, which is the voltage in tenths of millivolts
// For example, if voltage_tmv == 119841, that corresponds to a voltage of 11.9841 volts.  This function only
// prints up to two decimal points.
void get_voltage_string( int32_t voltage_tmv, char* output_buf )
{
	int string_length;
	char string_buf[10];
	
	ConvertItoA(voltage_tmv, string_buf, 10);
	
	string_length = 0;
	while( (string_length < 10) && (string_buf[string_length] != '\0') )
	{
		string_length++;
	}

	/* 
		Fill the output buffer depending on the length of the string buffer
		
		If greater than equal to five digits, then we have something like
		11984, which needs to become 11.98.  If the string length is L, then 
		start by copying out (L-4) digits.  In this case, copy out 2 digits:
			"11"
		Now copy out a decimal point:
			"11."
		Finally, copy out the final two digits:
			"11.98"
			
		If 4 digits, we have something like
		1198, which needs to become 0.11 (or 0.12 if we decide to round...)  Start by
		writing "0." to the output.  Then write the next two digits:
			"0.11"
		Done!
		
		If only 3 digits, we have something like 119 which needs to become 0.01.  Start by writing
		"0.0" to the output.  Then write the first digit.  Done.
		
		If 2 or less digits, just write "0.00" to the output.
			
	*/
	
	if( string_length == 6 )
	{
		output_buf[0] = string_buf[0];
		output_buf[1] = string_buf[1];
		output_buf[2] = '.';
		output_buf[3] = string_buf[2];
		output_buf[4] = string_buf[3];
		output_buf[5] = '\0';
	}
	else if( string_length == 5)
	{
		output_buf[0] = string_buf[0];
		output_buf[1] = '.';
		output_buf[2] = string_buf[1];
		output_buf[3] = string_buf[2];
		output_buf[4] = '\0';
	}
	else if( string_length == 4)
	{
		output_buf[0] = '0';
		output_buf[1] = '.';
		output_buf[2] = string_buf[0];
		output_buf[3] = string_buf[1];
		output_buf[4] = '\0';
	}
	else if( string_length == 3)
	{
		output_buf[0] = '0';
		output_buf[1] = '.';
		output_buf[2] = '0';
		output_buf[3] = string_buf[0];
		output_buf[4] = '\0';
	}
	else
	{
		output_buf[0] = '0';
		output_buf[1] = '.';
		output_buf[2] = '0';
		output_buf[3] = '0';
		output_buf[4] = '\0';
	}
}

