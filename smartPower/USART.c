/*
 * USART.c
 *
 * Created: 12/6/2012 4:35:56 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "USART.h"
#include "config.h"
#include "util.h"

volatile char g_RX_buf[RX_BUF_SIZE];
volatile uint8_t g_RX_size;

volatile char g_packet_buf[RX_BUF_SIZE];
volatile uint8_t g_packet_size;
volatile uint8_t g_new_packet;

// Circular TX buffer
volatile char g_TX_buf[TX_BUF_SIZE];
volatile uint8_t g_TX_start;
volatile uint8_t g_TX_end;

volatile uint8_t g_binary_mode;
volatile uint8_t g_USART_state;

volatile Packet g_received_packet;

// Initialize the USART
void usart_init()
{
	g_binary_mode = 0;
	g_RX_size = 0;
	g_TX_start = 0;
	g_TX_end = 0;
	g_new_packet = 0;
	
	// Enable double USART transmission speed
	//	UCSR0A = 0x02;
	
	// Turn off receiver and transmitter and disable interrupts
	UCSR0B = 0x00;
	
	// Normal asynchronous mode, no parity, 1 stop bit
	UCSR0C = 0x06;
	
	// Baud rate register.
	/*
		At 10 Mhz, w/UBRR0H = 0x00
		UBRR0L		Desired Baud	Actual		% Error
		4			115200			125000		+8.5%
		8			76800			69444		-9.6%
		11			57600			52083		-9.5%
		16			38400			36765		-4.3%
		22			28800			27174		-5.6%
		33			19200			18382		-4.3%
		43			14400			14205		-1.4%
		65			9600			9470		-1.4%
	*/
	UBRR0H = 0x00;
	UBRR0L = 65;
	
	// Enable receiver and transmitter and relevant interrupts
	//	UCSR0B |= ;	// Interrupt when transmit data register is empty (transmission may still be in progress)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << UDRIE0);		// Enable receiver and transmitter, and turn on the RX and TX interrupts
	
	// Global interrupts are not enabled here.  Must be done before USART will work properly.
}

// Function for transmitting a null-terminated sequence of characters.
void USART_transmit(char* buf)
{
	uint8_t length = strlen(buf);
	
	USART_transmit_length(buf, length);
}

// Function for transmitting a null-terminated sequence of characters.  Doesn't return until the transmission is complete.
void USART_transmit_block(char* buf)
{
	USART_transmit(buf);
	
	while( !USART_is_ready() );
}

// Function for transmitting a sequence of characters that is not null-terminated
void USART_transmit_length(volatile char* buf, uint8_t length)
{
	if( length == 0 || (length >=  TX_BUF_SIZE) )
	{
		return;
	}
	
	// Push onto buffer...
	uint8_t index;
	for( index = 0; index < length; index++ )
	{
		TX_buf_add(buf[index]);
	}
	
	// If the transmitter isn't already operating, initiate transmission
	USART_send_next();	
}

// Add character to circular TX buffer
void TX_buf_add( char data )
{
	// Do a quick bounds check to make sure that g_TX_end is not somehow too large.
	// This should never be a problem, but... just to make sure.  If this happens,
	// reset the entire TX circular buffer.  This may cause weird behavior, as data
	// being transmitted is unpredictably truncated.  But if that is happening because
	// of the reset here, then there is a different problem somewhere else that is
	// screwing with the tx buffer pointers.
	if( g_TX_end >= TX_BUF_SIZE )
	{
		g_TX_end = 0;
		g_TX_start = 0;
	}
	
	g_TX_buf[g_TX_end] = data;
	
	g_TX_end++;
	
	// Wrap g_TX_end to the beginning of the TX buffer
	if( g_TX_end >= TX_BUF_SIZE )
	{
		g_TX_end = 0;
	}
	
}

// Function for sending the next data in the circular TX buffer
void USART_send_next()
{
	// If the circular buffer is empty, return
	if( g_TX_start == g_TX_end )
	{
		return;
	}
	
	// If USART is already operating, return
	if( !USART_is_ready() )
	{
		return;
	}
	
	UDR0 = g_TX_buf[g_TX_start];
	
	g_TX_start++;
	
	if( g_TX_start >= TX_BUF_SIZE )
	{
		g_TX_start = 0;
	}
}

// Function call for determining whether the USART transmitter is active
uint8_t USART_is_ready()
{
	if( (UCSR0A & (1 << UDRE0)) == 0 )
	{
		return 0;
	}
	
	return 1;
}

// USART RX interrupt routine
//__attribute__((optimize("O3")))
ISR(USART_RX_vect)
{
	char ReceivedByte;
	static uint8_t data_received = 0;
	
	// Handle buffer overflow
	if( g_RX_size >= (RX_BUF_SIZE-1) )
	{
		g_RX_size = 0;
	}
	
	// Read the received byte.
	ReceivedByte = UDR0;
	
	// If we are in binary mode, process incoming data as binary packets
	if( g_binary_mode )
	{		
		switch( g_USART_state )
		{
			// Idle state
			case STATE_USART_IDLE:
				g_RX_buf[g_RX_size++] = ReceivedByte;
			
				// Watch for :x sequence for moving back to ascii mode
				if( (data_received == 0) && ReceivedByte == ':' )
				{
					data_received++;
				}
				else if( data_received == 1 )
				{
					if( ReceivedByte == 'x' )
					{
						g_RX_size = 0;
						g_binary_mode = 0;
						USART_transmit("\r\nEntering ASCII mode.\r\n>");
					}
					
					data_received = 0;
				}
				
				// Watch for 'snp' sequence
				if( g_RX_size >= 3 )
				{
					if( (g_RX_buf[g_RX_size-3] == 's') && (g_RX_buf[g_RX_size-2] == 'n') && (g_RX_buf[g_RX_size-1] == 'p'))
					{
						g_RX_size = 0;
						g_USART_state = STATE_USART_PACKET_TYPE;
						data_received = 0;
					}
				}				
			break;
			
			case STATE_USART_PACKET_TYPE:
				g_received_packet.type = ReceivedByte;
				
				// Set the data length using the packet type byte
				if( (g_received_packet.type & PACKET_HAS_DATA) == 0 )
				{
					// No data
					g_received_packet.data_length = 0;
				}
				// There is data in this packet
				else
				{
					// Check for batch packet
					if( g_received_packet.type & PACKET_IS_BATCH )
					{
						// This is a batch packet.  Extract the batch length.
						g_received_packet.batch_length = (g_received_packet.type >> PACKET_BATCH_LENGTH_OFFSET) & PACKET_BATCH_LENGTH_MASK;
						g_received_packet.data_length = 4*g_received_packet.batch_length;
					}
					else
					{
						g_received_packet.batch_length = 0;
						g_received_packet.data_length = 4;
					}
				}
				
				// Switch to the next state.  If the data length was invalid, return to idle state
				if( g_received_packet.data_length > MAX_PACKET_DATA_LENGTH )
				{
					g_USART_state = STATE_USART_IDLE;
				}
				else
				{
					g_USART_state = STATE_USART_ADDRESS;
				}
			break;			
			
			case STATE_USART_ADDRESS:
				g_received_packet.address = ReceivedByte;
				
				// Set the address type (for convenience in working with the data later)
				if( g_received_packet.address >= COMMAND_START_ADDRESS )
				{
					g_received_packet.address_type = ADDRESS_TYPE_COMMAND;
				}
				else if( g_received_packet.address > DATA_REG_START_ADDRESS )
				{
					g_received_packet.address_type = ADDRESS_TYPE_DATA;
				}
				else
				{
					g_received_packet.address_type = ADDRESS_TYPE_CONFIG;
				}
				
				// Switch to the next state
				if( g_received_packet.data_length > 0 )
				{
					data_received = 0;
					g_USART_state = STATE_USART_DATA;
				}
				else
				{
					g_USART_state = STATE_USART_CHECKSUM;
				}
				
			break;
			
			case STATE_USART_DATA:
				g_received_packet.data[data_received++] = ReceivedByte;
				
				if( data_received == g_received_packet.data_length )
				{
					data_received = 0;
					g_USART_state = STATE_USART_CHECKSUM;
				}
			break;
			
			
			case STATE_USART_CHECKSUM:
				if( data_received == 0 )
				{
					g_received_packet.checksum = ((uint16_t)ReceivedByte) << 8;
					data_received++;
				}
				else
				{
					g_received_packet.checksum |= ReceivedByte;
					
					uint16_t checksum = compute_checksum((Packet*)&g_received_packet);
					
					if( checksum == g_received_packet.checksum )
					{
						// Checksum matches!  We've received a good packet.
						g_new_packet = 1;
						data_received = 0;
						g_USART_state = STATE_USART_IDLE;
					}
					else
					{
						// We got a full packet, but the checksums didn't match
						
						data_received = 0;
						g_USART_state = STATE_USART_IDLE;
						
						/*
						// TODO: Send a message or something
						USART_transmit_block("Bad Checksum!\r\n");
						TX_buf_add((char)(checksum >> 8));
						TX_buf_add((char)(checksum & 0x0FF));
						TX_buf_add((char)(g_received_packet.checksum >> 8));
						TX_buf_add((char)(g_received_packet.checksum & 0x0FF));
						
						USART_send_next();
						*/
					}
				}
				
			break;
			
		}
		
	}
	// In normal ascii mode, echo received characters and watch for carriage return
	else
	{		
		// Check for backspace
		if( ReceivedByte == 0x08 || ReceivedByte == 0x7F )
		{
			if( g_RX_size > 0 )
			{
				g_RX_size--;
				
				// Echo what we received
				if( !g_refreshing_display )
				{
					TX_buf_add( ReceivedByte );
					USART_send_next();
				}
			}
		}
		else if( ((ReceivedByte >= 32) && (ReceivedByte <= 126)) || (ReceivedByte == '\r'))
		{
			if( ReceivedByte == '\r' )
			{
				/* Turned off for new display methodology.  Using VT-100 clear screen and cursor movement commands, data entry always happens on a single line now
				if( !g_refreshing_display )
				{
					TX_buf_add('\r');
					TX_buf_add('\n');
					USART_send_next();
				}
				*/
			}
			else
			{
				// Echo what we received
				if( !g_refreshing_display )
				{
					TX_buf_add( ReceivedByte );
					USART_send_next();
				}
			}
			
			g_RX_buf[g_RX_size++] = ReceivedByte;
			
			// If this was a carriage return, copy all the data to the packet buffer and set a flag indicating that
			// a packet was received
			if( (g_RX_buf[g_RX_size-1] == '\r') )
			{
				uint8_t index;
				for( index = 0; index < g_RX_size; index++ )
				{
					g_packet_buf[index] = g_RX_buf[index];
				}
				
				// Add null terminating character so we can treat this like a string
				g_packet_buf[g_RX_size] = '\0';
				
				g_new_packet = 1;
				g_packet_size = g_RX_size;
				
				g_RX_size = 0;
			}
		}
	}
	
	
}

// USART TX interrupt routine
ISR(USART_UDRE_vect)
{	
	// If there is more data to be transmitted, send it
	USART_send_next();
}

/*******************************************************************************
* Function Name  : compute_checksum
* Input          : Packet* new_packet
* Output         : None
* Return         : uint16_t
* Description    : Returns the two byte sum of all the individual bytes in the
						 given packet.
*******************************************************************************/
uint16_t compute_checksum( Packet* new_packet )
{
	 int32_t index;

	 uint16_t checksum = (uint16_t)0x0073 + (uint16_t)0x006E + (uint16_t)0x0070 + (uint16_t)new_packet->type + (uint16_t)new_packet->address;
	 
	 for( index = 0; index < new_packet->data_length; index++ )
	 {
		  checksum += new_packet->data[index];
	 }
	 
	 return checksum;
}


/*******************************************************************************
* Function Name  : transmit_packet
* Input          : Packet* new_packet
* Output         : None
* Return         : void
* Description    : Adds the specified packet to the TX buffer for transmission
*******************************************************************************/
void transmit_packet( Packet* new_packet )
{
	int index;
	
	// Make sure the packet length isn't too large to fit
	if( new_packet->data_length >= MAX_PACKET_DATA_LENGTH )
	{
		return;
	}
	
	TX_buf_add( 's' );
	TX_buf_add( 'n' );
	TX_buf_add( 'p' );
	
	TX_buf_add( new_packet->type );
	TX_buf_add( new_packet->address );

	for( index = 0; index < new_packet->data_length; index++ )
	{
		TX_buf_add(new_packet->data[index]);
	}
	
	TX_buf_add( (new_packet->checksum >> 8) & 0x0FF );
	TX_buf_add( new_packet->checksum & 0x0FF );
	
	USART_send_next();
}
