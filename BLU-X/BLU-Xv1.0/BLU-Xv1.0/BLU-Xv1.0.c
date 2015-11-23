/*
 * BLU_Xv1.c
 *
 * Created: 4/9/2013 11:11:22 AM
 *  Author: Zack
 */ 

#include <avr/io.h>
#include "asf.h"
#include "USART.h"
#include "config.h"
#include "BLU-Xv1.0.h"

volatile uint8_t g_devicePaired;
volatile uint8_t g_request_BLE_status;

uint8_t g_new_BLE_status_received;
uint8_t g_BLE_status;
int main(void)
{
	int index;
	
	g_devicePaired = 0;
	g_request_BLE_status = 0;
	g_new_BLE_status_received = 0;
	g_BLE_status = 0;
	
	g_hydra_binary_mode = 0;
	g_hydra_new_packet = 0;
	
	g_BLE_new_packet = 0;
	
	PUEB = 0x00;
	DDRB = (1<<PORTB3);
	PORTB |= BLU_LED;
		
	UCSR0A = 0x00;
	UCSR1A = 0x00;
	
	// Configure the 16-bit hardware timer for handling scheduled events
	TCCR1A = 0x01;		// 0b00000001
	TCCR1C = 0x00;
	// Set output capture register A
	OCR1A = (uint16_t)156;
	
	// Enable output capture interrupt
	TIMSK |= (1 << OCIE1A);
	// Set the timer clock source to enable it.
	TCCR1B = 0x05;		// 0b00000101
	
	// Enable global interrupts
	SREG = 0x80;
	
	// Initialize the BLE UART
	USART_BLE_init();
	USART_hydra_init();
	
	/*
	// Wait for Bluetooth module to initialize
	
	_delay_ms(1600);
	
	// Configure the BLE device and attempt to establish a connection.
	// If at any time a command fails, flash the indicator LED quickly
	// to indicate where the failure took place.  Then hang indefinitely.
	// Trying again requires the device to be power cycled.
	USART_transmit_block("BC:FD\r\n", BLE_USART);
	// No response check.  There is no response to a factory reset.

	_delay_ms(100);
	
	// Clear list of bluetooth connections
	
	USART_transmit_block("BC:CP\r\n", BLE_USART);
	returnval = BLU_check_command();
	if( returnval )
	{
		fast_heart_beat(returnval);
	}
	
	_delay_ms(100);
	*/
	
	// Wait a second for the Hydra to power up
	_delay_ms(1000);
	
	// Send a message to the Hydra to switch to binary broadcast mode.
	USART_transmit_block(":b\r", HYDRA_USART);
		
	PORTB |= BLU_LED;
	_delay_ms(100);

	int toggle = 0;
    while(1)
    {
		// If we aren't connected, this function call will block until a connection is established. It will also handle
		// COM with the Bluetooth modem to get it into the correct state if needed.
		// If we are connected, this function call does nothing and returns immediately
//		service_BLU_connection();
		
		// Request the radio status once per second
		if( g_request_BLE_status )
		{
			g_request_BLE_status = 0;
			
			// If we aren't paired, toggle the indicator LED every time we transmit a status request packet to the BLE module
			if( !g_devicePaired )
			{
				if( toggle )
				{
					PORTB |= BLU_LED;
					toggle = 0;
				}
				else
				{
					PORTB &= ~BLU_LED;
					toggle = 1;
				}
			}
			
			// Transmit the status request
			USART_transmit_block("BC:RT\r\n", BLE_USART);
		}
		
		// Service BLE packets if we've received one
		if( g_BLE_new_packet )
		{
			g_BLE_new_packet = 0;
			
			process_BLE_packet();
		}
		
		// Check for a change in the BL module status
		if( g_new_BLE_status_received )
		{
			g_new_BLE_status_received = 0;
			
			// If status == 0, the radio is inactive and not available for connection.  Make it discoverable.
			if( g_BLE_status == 0 )
			{
				BLU_make_discoverable();
				g_devicePaired = 0;
			}
			// If status equal 5, we are connected.
			else if( g_BLE_status == 5 )
			{
				// Turn on link LED
				PORTB &= ~BLU_LED;
				
				g_devicePaired = 1;
			}
			// If we ever encounter status 4, it likely means that a different Bluetooth central is trying to connect.  Do a factory reset
			// to allow a new connection to be established.
			else if( g_BLE_status == 4 )
			{
				BLU_factory_reset();
				g_devicePaired = 0;
			}
		}
		
		// If we are paired, forward data from the Hydra through the Bluetooth module
		if( g_devicePaired )
		{
			// If we've received a packet from the Hydra, send it through the Bluetooth connection
			if(g_hydra_new_packet)
			{
				g_hydra_new_packet = 0;
				
				// Take all the packet data and re-transmit it through the Bluetooth module
				char Hydra_buf[60];
				
				// At one time, we can only transmit 20 bytes of data.  The packets from the Hydra tend to be longer than that.
				// If received_packet.data_length > 12, then the packet will be greater than 20 bytes in length (= 7 + data_length + \r\n).
				// Add as many bytes to the buffer as possible and then transmit the rest.
				Hydra_buf[0] = 'B';
				Hydra_buf[1] = 'C';
				Hydra_buf[2] = ':';
				Hydra_buf[3] = 'D';
				Hydra_buf[4] = 'T';
				Hydra_buf[5] = '=';
				Hydra_buf[6] = 's';
				Hydra_buf[7] = 'n';
				Hydra_buf[8] = 'p';
				Hydra_buf[9] = g_hydra_received_packet.type;							// Packet type (0xCC = 204 = 0b11001100)
				Hydra_buf[10] = g_hydra_received_packet.address;						// Address
				
				// Handle long packets
				if( g_hydra_received_packet.data_length > 12 )
				{
					uint16_t checksum = 0;
					uint8_t escape_offset = 0;
					for( index = 0; index < 12; index++ )
					{
						// Check for carriage return,linefeed, and backslash.  These need to be escaped to be sent properly
						if( needs_escape(g_hydra_received_packet.data[index]) )
						{
							Hydra_buf[11+index+escape_offset] = 0x5C;
							escape_offset++;
						}
						Hydra_buf[11+index+escape_offset] = g_hydra_received_packet.data[index];
						checksum += g_hydra_received_packet.data[index];
					}
					
					Hydra_buf[23+escape_offset] = '\r';
					Hydra_buf[24+escape_offset] = '\n';
					
					// Transmit what we have so far
					USART_transmit_length_block(Hydra_buf,25+escape_offset,BLE_USART);
					
					// Now transmit the rest of the stuff...
					escape_offset = 0;
					for( index = 12; index < g_hydra_received_packet.data_length; index++ )
					{
						// Check for carriage return,linefeed, and backslash.  These need to be escaped to be sent properly
						if( needs_escape(g_hydra_received_packet.data[index]) )
						{
							Hydra_buf[6+(index-12)+escape_offset] = 0x5C;
							escape_offset++;
						}
						
						// This only works because index starts at 12!
						Hydra_buf[6+(index-12)+escape_offset] = g_hydra_received_packet.data[index];
						checksum += g_hydra_received_packet.data[index];
					}
					
					uint8_t checksum_offset = (g_hydra_received_packet.data_length - 12);
					
					checksum += (115 + 110 + 112 + g_hydra_received_packet.type + g_hydra_received_packet.address);
					Hydra_buf[6+checksum_offset+escape_offset] = (checksum >> 8);
					Hydra_buf[7+checksum_offset+escape_offset] = (checksum & 0x0FF);
					Hydra_buf[8+checksum_offset+escape_offset] = '\r';
					Hydra_buf[9+checksum_offset+escape_offset] = '\n';
					
					// Transmit the rest
					USART_transmit_length_block(Hydra_buf,10+checksum_offset+escape_offset,BLE_USART);
				}
				// Handle short packets
				else
				{
					uint16_t checksum = 0;
					uint8_t escape_offset = 0;
					for( index = 0; index < g_hydra_received_packet.data_length; index++ )
					{
						// Check for carriage return,linefeed, and backslash.  These need to be escaped to be sent properly
						if( needs_escape(g_hydra_received_packet.data[index]) )
						{
							Hydra_buf[11+index+escape_offset] = 0x5C;
							escape_offset++;
						}
						
						Hydra_buf[11+index+escape_offset] = g_hydra_received_packet.data[index];
						checksum += g_hydra_received_packet.data[index];
					}
					checksum += (115 + 110 + 112 + g_hydra_received_packet.type + g_hydra_received_packet.address);
					Hydra_buf[11+g_hydra_received_packet.data_length+escape_offset] = (checksum >> 8);
					Hydra_buf[12+g_hydra_received_packet.data_length+escape_offset] = (checksum & 0x0FF);
					Hydra_buf[13+g_hydra_received_packet.data_length+escape_offset] = '\r';
					Hydra_buf[14+g_hydra_received_packet.data_length+escape_offset] = '\n';
					
					USART_transmit_length_block(Hydra_buf,15+g_hydra_received_packet.data_length+escape_offset,BLE_USART);
				}
				
				// Now that we've transmitted the response, flicker the LED
				PORTB |= BLU_LED;
				_delay_ms(20);
				PORTB &= ~BLU_LED;
			}
		}
		
    }
}

// Returns 1 if the character needs to be escaped to be sent, 0 otherwise
int needs_escape( char data )
{
	if( data == 0x0D || data == 0x0A || data == 0x5C )
	{
		return 1;
	}
	
	return 0;
}

// Function call for processing Bluetooth packets as they arrive.
// If it receives a status packet, the global status variable is updated
// so that the main function loop can do something about it.
// If a data packet is received that needs to be forwarded, it
// takes care of the forwarding
void process_BLE_packet()
{
	int index;
	int packet_start = -1;
	
	// Check for status packet.  This is signified by the text "RT="
	if( g_BLE_packet_size >= 5 )
	{
		// First, find the first character of the expected connection packet.  We do this to ensure that
		// any non-packet characters that may have been transmitted or erroneously read by the UART hardware
		// does not interfere with decoding the packet.
		for(index = 0; index < (g_BLE_packet_size - 5); index++ )
		{
			if( (g_BLE_packet_buf[index+0] == 'R') &&
			(g_BLE_packet_buf[index+1] == 'T') &&
			(g_BLE_packet_buf[index+2] == '=') )
			{
				// Found a status packet.
				packet_start = index;
				break;
			}
		}
		
		if( packet_start > -1 )
		{
			// If packet_start > -1, we found the character sequence "RT="
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '0' )
			{
				g_BLE_status = 0;
				g_new_BLE_status_received = 1;
			}
			else if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '1' )
			{
				g_BLE_status = 1;
				g_new_BLE_status_received = 1;
			}
			else if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '2' )
			{
				g_BLE_status = 2;
				g_new_BLE_status_received = 1;
			}
			else if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '3' )
			{
				g_BLE_status = 3;
				g_new_BLE_status_received = 1;
			}
			else if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '4' )
			{
				g_BLE_status = 4;
				g_new_BLE_status_received = 1;
			}
			else if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '5' )
			{
				g_BLE_status = 5;
				g_new_BLE_status_received = 1;
			}
			
			return;
		}
		
		// Now search for the 'snp' sequence indicating the start of a new packet destined for the Hydra
		for(index = 0; index < (g_BLE_packet_size - 5); index++ )
		{
			if( (g_BLE_packet_buf[index+0] == 's') &&
			(g_BLE_packet_buf[index+1] == 'n') &&
			(g_BLE_packet_buf[index+2] == 'p') )
			{
				// Found a status packet.
				packet_start = index;
				break;
			}
		}
		
		// If 'snp' was found, then take the packet and forward it to the Hydra USART
		if( packet_start > -1 )
		{
			uint8_t packet_type = g_BLE_packet_buf[packet_start+3];
			uint8_t batch_length;
			uint8_t data_length;
			
			// Compute the length and make sure that we have enough data for a full packet
			// Check for batch packet
			if( packet_type & PACKET_HAS_DATA )
			{
				if( packet_type & PACKET_IS_BATCH )
				{
					// This is a batch packet.  Extract the batch length.
					batch_length = (packet_type >> PACKET_BATCH_LENGTH_OFFSET) & PACKET_BATCH_LENGTH_MASK;
					data_length = 4*batch_length;
				}
				else
				{
					batch_length = 0;
					data_length = 4;
				}
			}
			// No data in this packet
			else
			{
				batch_length = 0;
				data_length = 0;
			}
			
			// Check for length.  If there is enough data, send it to the USART.
			if( (data_length + 7) <= (g_BLE_packet_size - packet_start) )
			{	
				USART_transmit_length(&g_BLE_packet_buf[packet_start],data_length + 7, HYDRA_USART);
			}
			
		}
		
	}
	
	return;
}

// If the Bluetooth module isn't paired, waits to receive a connection
void service_BLU_connection()
{
	int connection_status;
	
	// If the device isn't paired, toggle the output LED once per second and attempt to connect...
	int toggle = 0;
	while( !g_devicePaired )
	{
		USART_transmit_block("BC:RT\r\n", BLE_USART);
		_delay_ms(1000);
		if( toggle )
		{
			PORTB |= BLU_LED;
			toggle = 0;
		}
		else
		{
			PORTB &= ~BLU_LED;
			toggle = 1;
		}

		// Get the connection status.  This simply checks to see if there is a new packet from the bluetooth radio indicating
		// the connection status.  If no packet is available, then the function returns -1
		connection_status = BLU_get_connection_status();
		
		// If status indicates that we are connected, leave the loop
		if(connection_status == 5 )
		{
			// Turn on link LED
			PORTB &= ~BLU_LED;
			
			g_devicePaired = 1;
		}
		// If we ever encounter status 4, it likely means that a different Bluetooth central is trying to connect.  Do a factory reset
		// to allow a connection to be established.
		else if( connection_status == 4 )
		{
			BLU_factory_reset();
		}
		// If we are ever in idle mode, send a packet to have the bluetooth radio transition to discoverable mode
		else if( connection_status == 0 )
		{
			BLU_make_discoverable();
		}
	}
}

// Sends a packet to the Bluetooth module to change it to discoverable mode
void BLU_make_discoverable()
{
	// Make discoverable
	USART_transmit_block("BC:MD=01\r\n", BLE_USART);	
	_delay_ms(100);
}

// Sends a packet to the Bluetooth module to initiate a factory reset
void BLU_factory_reset()
{
	USART_transmit_block("BC:FD\r\n", BLE_USART);
	_delay_ms(100);
}

// Function call watches for a packet with RT=NN, where NN is the status
// Returns -1 if no status packet was available
int BLU_get_connection_status( )
{
	int index;
	int packet_start;
	
	packet_start = -1;
	// Watch for a packet indicating that we've connected
	if( g_BLE_new_packet )
	{
		g_BLE_new_packet = 0;
		
		// Check for "RT="
		if( g_BLE_packet_size >= 5 )
		{
			// First, find the first character of the expected connection packet.  We do this to ensure that
			// any non-packet characters that may have been transmitted or erroneously read by the UART hardware
			// does not interfere with decoding the packet.
			for(index = 0; index < (g_BLE_packet_size - 5); index++ )
			{
				if( (g_BLE_packet_buf[index+0] == 'R') &&
					(g_BLE_packet_buf[index+1] == 'T') &&
					(g_BLE_packet_buf[index+2] == '=') )
					{
						// Found a status packet.
						packet_start = index;
						break;
					}
			}
			
			// If packet_start > -1, we found the character sequence "RT="
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '0' )
			{
				return 0;
			}
			
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '1' )
			{
				return 1;
			}
			
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '2' )
			{
				return 2;
			}
			
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '3' )
			{
				return 3;
			}
			
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '4' )
			{
				return 4;
			}
			
			if( g_BLE_packet_buf[packet_start+3] == '0' && g_BLE_packet_buf[packet_start+4] == '5' )
			{
				return 5;
			}
			
		}
	}
	
	// No packet was available for parsing
	return -1;	
}

// Waits for a response from the BLU-X module.  If no response in a certain time period, returns an error.
int BLU_wait_for_response( )
{
	int timeout = 0;
	
	while( timeout < 20 )
	{
		timeout++;
		
		_delay_ms(100);
		if( g_BLE_new_packet )
		{
			return 0;
		}
	}
	
	// If function hasn't returned yet, it means that the Bluetooth module has not responded after a full second.
	// Return 1 to indicate an error condition.
	return 1;
}

// Watches for "OK" packet from Bluetooth module.  If OK not received, returns an error
int BLU_check_command( )
{
	// Wait to receive a packet
	if( BLU_wait_for_response() )
	{
		return 3;
	}
	
	// Clear the BLE new packet flag
	g_BLE_new_packet = 0;
	
	// Once here, we know we have a packet.  Check to make sure it says "OK"
	if( (g_BLE_packet_buf[0] == 'O') && (g_BLE_packet_buf[1] == 'K') )
	{
		return 0;
	}
	
//	USART_transmit_length(g_BLE_packet_buf, g_BLE_packet_size, HYDRA_USART);
	
	// If we get here, we received a packet in response, but the packet did not say "OK"  This indicates
	// a command failure.  Return 1 to indicate an error.
	return 4;
}

void heart_beat(uint8_t beatCount)
{
	uint8_t i;
	uint8_t portBState = PORTB;

	for(i = 0; i < beatCount; i++)
	{
		PORTB &= ~BLU_LED;
		_delay_ms(1000);
		PORTB |= BLU_LED;
		_delay_ms(1000);
	}		
	
	//restore PortB
	PORTB = portBState;
}

void fast_heart_beat(uint8_t beatCount)
{
	uint8_t i;
	uint8_t portBState = PORTB;
	
	for(i = 0; i < beatCount; i++)
	{
		PORTB &= ~BLU_LED;
		_delay_ms(200);
		PORTB |= BLU_LED;
		_delay_ms(200);
	}
	
	//restore PortB
	PORTB = portBState;
}

// Timer interrupt service vector
ISR(TIMER1_COMPA_vect)
{
	static int counter = 0;
	
	counter++;

	if( counter == 32 )
	{
		// This ISR should be called once per second.  Have it send a request for the BLE module status.
		g_request_BLE_status = 1;
		
		counter = 0;
	}
}
