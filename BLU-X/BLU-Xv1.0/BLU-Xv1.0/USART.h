/*
 * USART.h
 *
 * Created: 4/11/2013 8:51:09 AM
 *  Author: Zack
 */ 


#ifndef USART_H_
#define USART_H_

//USART ID
#define HYDRA_USART		0x1
#define BLE_USART		0x2

#define HYDRA_RX_BUF_SIZE	30
#define HYDRA_TX_BUF_SIZE 150

#define BLE_RX_BUF_SIZE	30
#define BLE_TX_BUF_SIZE 150

// State definitions for binary mode on serial port
#define STATE_USART_IDLE			0
#define STATE_USART_PACKET_TYPE		1
#define STATE_USART_ADDRESS			2
#define STATE_USART_DATA			3
#define STATE_USART_CHECKSUM		4

#define MAX_PACKET_DATA_LENGTH		20

#define	PACKET_HAS_DATA			(1 << 7)
#define	PACKET_IS_BATCH			(1 << 6)
#define	PACKET_BATCH_LENGTH_MASK	(0x0F)

#define	PACKET_BATCH_LENGTH_OFFSET	2

#define	BATCH_SIZE_2				2
#define	BATCH_SIZE_3				3

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
	char data[MAX_PACKET_DATA_LENGTH];
	uint8_t data_length;
	uint8_t batch_length;
	uint8_t address_type;
	uint16_t checksum;
} Packet;


void USART_transmit(char* buf, uint8_t USART_ID);							//*
void USART_transmit_block(char* buf, uint8_t USART_ID);						//*
void USART_transmit_length(char* buf, uint8_t length, uint8_t USART_ID);	//*
void USART_transmit_length_block(char* buf, uint8_t length, uint8_t USART_ID);
void USART_send_next(uint8_t USART_ID);										//*
void USART_hydra_init(void);												//*
void USART_BLE_init(void);													//*

uint8_t USART_is_ready(uint8_t USART_ID);									//*

void TX_buf_add(char data, uint8_t USART_ID);								//*

void transmit_packet(Packet* new_packet, uint8_t USART_ID);					//*
uint16_t compute_checksum(Packet* new_packet);

extern volatile Packet g_hydra_received_packet;
extern volatile Packet g_BLE_received_packet;

extern volatile char g_hydra_RX_buf[HYDRA_RX_BUF_SIZE];
extern volatile uint8_t g_hydra_RX_size;
extern volatile char g_BLE_RX_buf[BLE_RX_BUF_SIZE];
extern volatile uint8_t g_BLE_RX_size;

extern volatile char g_hydra_packet_buf[HYDRA_RX_BUF_SIZE];
extern volatile uint8_t g_hydra_packet_size;
extern volatile uint8_t g_hydra_new_packet;

extern volatile char g_BLE_packet_buf[BLE_RX_BUF_SIZE];
extern volatile uint8_t g_BLE_packet_size;
extern volatile uint8_t g_BLE_new_packet;

// Circular TX buffer
extern volatile char g_hydra_TX_buf[HYDRA_TX_BUF_SIZE];
extern volatile uint8_t g_hydra_TX_start;
extern volatile uint8_t g_hydra_TX_end;

extern volatile char g_BLE_TX_buf[BLE_TX_BUF_SIZE];
extern volatile uint8_t g_BLE_TX_start;
extern volatile uint8_t g_BLE_TX_end;

extern volatile uint8_t g_hydra_binary_mode;
extern volatile uint8_t g_BLE_binary_mode;





#endif /* USART_H_ */