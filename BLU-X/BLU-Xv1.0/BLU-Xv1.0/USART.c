/*
 * USART.c
 *
 * Created: 4/11/2013 8:50:35 AM
 *  Author: Zack
 */ 

#include "asf.h"
#include "USART.h"
#include "BLU-Xv1.0.h"
#include "config.h"

extern volatile uint8_t g_devicePaired;

volatile char g_hydra_RX_buf[HYDRA_RX_BUF_SIZE];
volatile uint8_t g_hydra_RX_size;
volatile char g_BLE_RX_buf[BLE_RX_BUF_SIZE];
volatile uint8_t g_BLE_RX_size;

volatile char g_hydra_packet_buf[HYDRA_RX_BUF_SIZE];
volatile uint8_t g_hydra_packet_size;
volatile uint8_t g_hydra_new_packet;
volatile char g_BLE_packet_buf[BLE_RX_BUF_SIZE];
volatile uint8_t g_BLE_packet_size;
volatile uint8_t g_BLE_new_packet;

// Circular TX buffer
volatile char g_hydra_TX_buf[HYDRA_TX_BUF_SIZE];
volatile uint8_t g_hydra_TX_start;
volatile uint8_t g_hydra_TX_end;
volatile char g_BLE_TX_buf[BLE_TX_BUF_SIZE];
volatile uint8_t g_BLE_TX_start;
volatile uint8_t g_BLE_TX_end;

volatile uint8_t g_hydra_binary_mode;
volatile uint8_t g_hydra_USART_state;
volatile uint8_t g_BLE_binary_mode;
volatile uint8_t g_BLE_USART_state;

volatile Packet g_hydra_received_packet;
volatile Packet g_BLE_received_packet;

// Initialize the Hydra USART
void USART_hydra_init()
{
	g_hydra_binary_mode = 0;
	g_hydra_RX_size = 0;
	g_hydra_TX_start = 0;
	g_hydra_TX_end = 0;
	g_hydra_new_packet = 0;
	
	g_hydra_USART_state = STATE_USART_IDLE;
	
	// Enable double USART transmission speed
	//	UCSR0A = 0x02;
	
	// Turn off receiver and transmitter and disable interrupts
	UCSR0B = 0x00;
	
	// Normal asynchronous mode, no parity, 1 stop bit
	UCSR0C = 0x06;
	
	// Baud rate register.
	/*
		At 8 Mhz, w/UBRR0H = 0x00
		UBRR0L		Desired Baud	Actual		% Error
		3			115200			125000		+8.5%
		6			76800			71429		-7.0%
		8			57600			55556		-3.5%
		12			38400			38462		+0.2%
		16			28800			29412		+2.1%
		25			19200			19231		+0.2%
		34			14400			14286		-0.8%
		51			9600			9615		+0.2%
	*/
	UBRR0H = 0x00;
	UBRR0L = 51;
	
	// Enable receiver and transmitter and relevant interrupts
	//	UCSR0B |= ;	// Interrupt when transmit data register is empty (transmission may still be in progress)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0) | (1 << RXCIE0) ;		// Enable receiver and transmitter, and turn on the RX and TX interrupts
	
	// Global interrupts are not enabled here.  Must be done before USART will work properly.
}


// Initialize the BLE USART
void USART_BLE_init()
{
	g_BLE_binary_mode = 0;
	g_BLE_RX_size = 0;
	g_BLE_TX_start = 0;
	g_BLE_TX_end = 0;
	g_BLE_new_packet = 0;
	
	g_BLE_USART_state = STATE_USART_IDLE;
	
	// Enable double USART transmission speed
	//	UCSR0A = 0x02;
	
	// Turn off receiver and transmitter and disable interrupts
	UCSR1B = 0x00;
	
	// Normal asynchronous mode, no parity, 1 stop bit
	UCSR1C = 0x06;
	
	// Baud rate register.
	/*
		At 8 Mhz, w/UBRR0H = 0x00
		UBRR0L		Desired Baud	Actual		% Error
		3			115200			125000		+8.5%
		6			76800			71429		-7.0%
		8			57600			55556		-3.5%
		12			38400			38462		+0.2%
		16			28800			29412		+2.1%
		25			19200			19231		+0.2%
		34			14400			14286		-0.8%
		51			9600			9615		+0.2%
		103			4800			4808		+0.2%
		207			2400			2404		+0.2%
	*/
	UBRR1H = 0x00;
	UBRR1L = 207;
	
	// Enable receiver and transmitter and relevant interrupts
	//	UCSR0B |= ;	// Interrupt when transmit data register is empty (transmission may still be in progress)
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1) | (1 << TXCIE1);		// Enable receiver and transmitter, and turn on the RX and TX interrupts
	
	// Global interrupts are not enabled here.  Must be done before USART will work properly.
}

// Function for transmitting a null-terminated sequence of characters.
void USART_transmit(char* buf, uint8_t USART_ID)
{
	uint8_t length = strlen(buf);
	
	USART_transmit_length(buf, length, USART_ID);
}

// Function for transmitting a null-terminated sequence of characters.  Doesn't return until the transmission is complete.
void USART_transmit_block(char* buf, uint8_t USART_ID)
{
	USART_transmit(buf, USART_ID);
	
	while( !USART_is_ready(USART_ID) );
}

// Function for transmitting a sequence of characters that is not null-terminated
void USART_transmit_length(char* buf, uint8_t length, uint8_t USART_ID)
{
	if(USART_ID == HYDRA_USART){
		if( length == 0 || (length >=  HYDRA_TX_BUF_SIZE) ){
			return;
		}
	}
	
	else if(USART_ID == BLE_USART){
		if( length == 0 || (length >=  BLE_TX_BUF_SIZE) ){
			return;
		}
	}
	
	// Push onto buffer...
	uint8_t index;
	for( index = 0; index < length; index++ )
	{
		TX_buf_add(buf[index], USART_ID);
	}
	
	// If the transmitter isn't already operating, initiate transmission
	USART_send_next(USART_ID);
}

// Function for transmitting a sequence of characters that is not null-terminated
// This blocks until all the data is transmitted
void USART_transmit_length_block(char* buf, uint8_t length, uint8_t USART_ID)
{
	USART_transmit_length(buf, length, USART_ID);
	
	while( !USART_is_ready(USART_ID) );	
}

// Function for sending the next data in the circular TX buffer
void USART_send_next(uint8_t USART_ID)
{
	// If the circular buffer of the defined USART is empty, return
	if(USART_ID == HYDRA_USART){
		if( g_hydra_TX_start == g_hydra_TX_end ){
			return;
		}
	}
	else if(USART_ID == BLE_USART){
		if( g_BLE_TX_start == g_BLE_TX_end ){
			return;
		}
	}		
	
	// If defined USART is already operating, return
	if(USART_ID == HYDRA_USART) {
		if( !USART_is_ready(USART_ID) )
		{
			return;
		}
	}
	else if(USART_ID == BLE_USART) {
		if( !USART_is_ready(USART_ID) )
		{
			return;
		}
	}
	if(USART_ID == HYDRA_USART) {
		UDR0 = g_hydra_TX_buf[g_hydra_TX_start];
		
		g_hydra_TX_start++;
		
		if( g_hydra_TX_start == HYDRA_TX_BUF_SIZE )
		{
			g_hydra_TX_start = 0;
		}
	}
	else if(USART_ID == BLE_USART) {
		UDR1 = g_BLE_TX_buf[g_BLE_TX_start];
		
		g_BLE_TX_start++;
		
		if( g_BLE_TX_start == BLE_TX_BUF_SIZE )
		{
			g_BLE_TX_start = 0;
		}
	}		
}

// Function call for determining whether the defined USART transmitter is active
uint8_t USART_is_ready(uint8_t USART_ID)
{
	if(USART_ID == HYDRA_USART) {
		if( (UCSR0A & (1 << UDRE0)) == 0 )
		{
			return 0;
		}
	}
	else if(USART_ID == BLE_USART){
		if( (UCSR1A & (1 << UDRE1)) == 0 )
		{
			return 0;
		}
	}
		
	return 1;
}

// Add character to defined circular TX buffer
void TX_buf_add( char data, uint8_t USART_ID )
{
	if(USART_ID == HYDRA_USART){
		g_hydra_TX_buf[g_hydra_TX_end] = data;
		
		g_hydra_TX_end++;
		
		if( g_hydra_TX_end == HYDRA_TX_BUF_SIZE )
		{
			g_hydra_TX_end = 0;
		}	
	}
	else if(USART_ID == BLE_USART){
		g_BLE_TX_buf[g_BLE_TX_end] = data;
		
		g_BLE_TX_end++;
		
		if( g_BLE_TX_end == BLE_TX_BUF_SIZE )
		{
			g_BLE_TX_end = 0;
		}
	}
}

/*******************************************************************************
* Function Name  : transmit_packet
* Input          : Packet* new_packet
* Output         : None
* Return         : void
* Description    : Adds the specified packet to the TX buffer for transmission
*******************************************************************************/
void transmit_packet( Packet* new_packet, uint8_t USART_ID )
{
	int index;
	
	TX_buf_add( 's', USART_ID );
	TX_buf_add( 'n', USART_ID );
	TX_buf_add( 'p', USART_ID );
	
	TX_buf_add( new_packet->type, USART_ID );
	TX_buf_add( new_packet->address, USART_ID );

	for( index = 0; index < new_packet->data_length; index++ )
	{
		TX_buf_add(new_packet->data[index], USART_ID);
	}
	
	TX_buf_add( (new_packet->checksum >> 8) & 0x0FF, USART_ID );
	TX_buf_add( new_packet->checksum & 0x0FF, USART_ID );
	
	USART_send_next(USART_ID);
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

	 uint16_t checksum = 0x73 + 0x6E + 0x70 + new_packet->type + new_packet->address;
	 
	 for( index = 0; index < new_packet->data_length; index++ )
	 {
		  checksum += new_packet->data[index];
	 }
	 
	 return checksum;
}


/************************************************************************/
/*						  USART ISRs                                    */
/************************************************************************/

// USART0 TX interrupt routine
ISR(USART0_TX_vect)
{
	// If there is more data to be transmitted, send it
	USART_send_next(HYDRA_USART);
}

// USART1 TX interrupt routine
ISR(USART1_TX_vect)
{
	// If there is more data to be transmitted, send it
	USART_send_next(BLE_USART);
}

// USART0 (Hydra) RX interrupt routine
ISR(USART0_RX_vect)
{ 
	static Packet new_packet;
	char ReceivedByte;
	static uint8_t data_received = 0;
	int index;
	
	// Handle buffer overflow
	if( g_hydra_RX_size == (HYDRA_RX_BUF_SIZE-1) )
	{
		g_hydra_RX_size = 0;
	}
	
	// Read the received byte.
	ReceivedByte = UDR0;
	
	switch( g_hydra_USART_state )
	{
		// Idle state
		case STATE_USART_IDLE:
			g_hydra_RX_buf[g_hydra_RX_size++] = ReceivedByte;
			
			// Watch for 'snp' sequence
			if( g_hydra_RX_size >= 3 )
			{
				if( (g_hydra_RX_buf[g_hydra_RX_size-3] == 's') && (g_hydra_RX_buf[g_hydra_RX_size-2] == 'n') && (g_hydra_RX_buf[g_hydra_RX_size-1] == 'p'))
				{
					g_hydra_RX_size = 0;
					g_hydra_USART_state = STATE_USART_PACKET_TYPE;
					data_received = 0;
					g_hydra_binary_mode = 1;
				}
			}
		break;
						
		case STATE_USART_PACKET_TYPE:
			new_packet.type = ReceivedByte;
			
			// Set the data length using the packet type byte
			if( (new_packet.type & PACKET_HAS_DATA) == 0 )
			{
				// No data
				new_packet.data_length = 0;
			}
			// There is data in this packet
			else
			{
				// Check for batch packet
				if( new_packet.type & PACKET_IS_BATCH )
				{
					// This is a batch packet.  Extract the batch length.
					new_packet.batch_length = (new_packet.type >> PACKET_BATCH_LENGTH_OFFSET) & PACKET_BATCH_LENGTH_MASK;
					new_packet.data_length = 4*new_packet.batch_length;
				}
				else
				{
					new_packet.batch_length = 0;
					new_packet.data_length = 4;
				}
			}
			
			// Switch to the next state.  If the data length was invalid, return to idle state
			if( new_packet.data_length > MAX_PACKET_DATA_LENGTH )
			{
				g_hydra_USART_state = STATE_USART_IDLE;
			}
			else
			{
				g_hydra_USART_state = STATE_USART_ADDRESS;
			}
		break;
			
		case STATE_USART_ADDRESS:
			new_packet.address = ReceivedByte;
			
			// Set the address type (for convenience in working with the data later)
			if( new_packet.address >= COMMAND_START_ADDRESS )
			{
				new_packet.address_type = ADDRESS_TYPE_COMMAND;
			}
			else if( new_packet.address > DATA_REG_START_ADDRESS )
			{
				new_packet.address_type = ADDRESS_TYPE_DATA;
			}
			else
			{
				new_packet.address_type = ADDRESS_TYPE_CONFIG;
			}
			
			// Switch to the next state
			if( new_packet.data_length > 0 )
			{
				g_hydra_USART_state = STATE_USART_DATA;
			}
			else
			{
				g_hydra_USART_state = STATE_USART_CHECKSUM;
			}
			
		break;
			
		case STATE_USART_DATA:
			new_packet.data[data_received++] = ReceivedByte;
			
			if( data_received == new_packet.data_length )
			{
				data_received = 0;
				g_hydra_USART_state = STATE_USART_CHECKSUM;
			}
		break;
			
			
		case STATE_USART_CHECKSUM:
			if( data_received == 0 )
			{
				new_packet.checksum = ReceivedByte << 8;
				data_received++;
			}
			else
			{
				new_packet.checksum |= ReceivedByte;
			
				// We just got the checksum.  The full packet has been received.  Revert to the idle state to wait for the next packet.	
				g_hydra_USART_state = STATE_USART_IDLE;
				data_received = 0;
				g_hydra_RX_size = 0;
				
				// Copy data out to global structure for use in the main program loop
				g_hydra_received_packet.address = new_packet.address;
				g_hydra_received_packet.address_type = new_packet.address_type;
				g_hydra_received_packet.type = new_packet.type;
				g_hydra_received_packet.data_length = new_packet.data_length;
				g_hydra_received_packet.checksum = new_packet.checksum;
				
				for( index = 0; index < new_packet.data_length; index++ )
				{
					g_hydra_received_packet.data[index] = new_packet.data[index];
				}
				
				// Set flag indicating that a new packet has arrived.
				g_hydra_new_packet = 1;
			}			
		break;			
	}	
}

// USART1 (BLE) RX interrupt routine
ISR(USART1_RX_vect)
{
	int index;
	char ReceivedByte;
	
	// Read the received byte.
	ReceivedByte = UDR1;
	
	g_BLE_RX_buf[g_BLE_RX_size++] = ReceivedByte;
		
	// Handle buffer overflow
	if( g_BLE_RX_size == (BLE_RX_BUF_SIZE-1) )
	{
		g_BLE_RX_size = 0;
	}
	
	// If we have received enough characters to form a full packet, check to see if we have received a carriage return and line-feed.
	// If so, copy the packet out and set a flag indicating that a packet has been received
	if( g_BLE_RX_size >= 2 )
	{
		if( (g_BLE_RX_buf[g_BLE_RX_size-2] == 0x0d) && (g_BLE_RX_buf[g_BLE_RX_size-1] == 0x0a) )
		{
			// Packet received.  Copy data out of USART RX buffer
			for( index = 0; index < g_BLE_RX_size; index++ )
			{
				g_BLE_packet_buf[index] = g_BLE_RX_buf[index];
			}
			g_BLE_packet_size = g_BLE_RX_size;
			
			// Clear USART RX buffer and set global flag indicating that a packet has been received
			g_BLE_RX_size = 0;
			g_BLE_new_packet = 1;
		}
	}
}
