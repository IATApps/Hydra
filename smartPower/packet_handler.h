/*
 * packet_handler.h
 *
 * Created: 2/3/2013 6:51:46 PM
 *  Author: Caleb
 */ 


#ifndef PACKET_HANDLER_H_
#define PACKET_HANDLER_H_

#include "config.h"
#include "USART.h"

void process_packet( Packet* new_packet );
void dispatch_packet( Packet* new_packet );

void send_command_success_packet( uint8_t command );
void send_command_failed_packet( uint8_t command );

void send_global_data(uint8_t address, uint8_t address_type, uint8_t is_batch, uint8_t batch_length);

#endif /* PACKET_HANDLER_H_ */