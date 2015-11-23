/*
 * USART.h
 *
 * Created: 12/6/2012 4:36:05 PM
 *  Author: Caleb
 */ 

#include <stdint.h>

#ifndef USART_H_
#define USART_H_


#define RX_BUF_SIZE	50
#define TX_BUF_SIZE 50

// State definitions for binary mode on serial port
#define STATE_USART_IDLE			0
#define STATE_USART_PACKET_TYPE		1
#define STATE_USART_ADDRESS			2
#define STATE_USART_DATA			3
#define STATE_USART_CHECKSUM		4

#define MAX_PACKET_DATA_LENGTH		20

#define	PACKET_HAS_DATA				(1 << 7)
#define	PACKET_IS_BATCH				(1 << 6)
#define	PACKET_BATCH_LENGTH_MASK	(0x0F)

#define	PACKET_BATCH_LENGTH_OFFSET	2

#define	BATCH_SIZE_2				2
#define	BATCH_SIZE_3				3
#define	BATCH_SIZE_4				4

#define	PACKET_NO_DATA				0
#define	PACKET_COMMAND_FAILED	(1 << 0)

// Define flags for identifying the type of packet address received
#define	ADDRESS_TYPE_CONFIG			0
#define	ADDRESS_TYPE_DATA			1
#define	ADDRESS_TYPE_COMMAND		2

#define	CONFIG_REG_START_ADDRESS	0
#define	DATA_REG_START_ADDRESS		85
#define	COMMAND_START_ADDRESS		170

// Structure for holding new packet data
typedef struct __packet_data
{
	uint8_t type;
	uint8_t address;
	uint8_t data[MAX_PACKET_DATA_LENGTH];
	uint8_t data_length;
	uint8_t batch_length;
	uint8_t address_type;
	uint16_t checksum;
} Packet;


void USART_transmit(char* buf);
void USART_transmit_block(char* buf);
void USART_transmit_length(volatile char* buf, uint8_t length);
void USART_send_next( void );
void usart_init( void );

uint8_t USART_is_ready( void );

void TX_buf_add( char data );

void transmit_packet( Packet* new_packet );
uint16_t compute_checksum( Packet* new_packet );

extern volatile Packet g_received_packet;

extern volatile char g_RX_buf[RX_BUF_SIZE];
extern volatile uint8_t g_RX_size;

extern volatile char g_packet_buf[RX_BUF_SIZE];
extern volatile uint8_t g_packet_size;
extern volatile uint8_t g_new_packet;

// Circular TX buffer
extern volatile char g_TX_buf[TX_BUF_SIZE];
extern volatile uint8_t g_TX_start;
extern volatile uint8_t g_TX_end;

extern volatile uint8_t g_binary_mode;

#endif /* USART_H_ */