/*
 * BLU_Xv1.h
 *
 * Created: 4/11/2013 2:10:03 PM
 *  Author: Zack
 */ 


#ifndef BLU_XV10_H_
#define BLU_XV10_H_

void heart_beat(uint8_t beatCount);
void fast_heart_beat(uint8_t beatCount);
int BLU_wait_for_response( void );
int BLU_check_command( void );
int BLU_get_connection_status( void );
void service_BLU_connection( void );
void BLU_make_discoverable( void );
void BLU_factory_reset( void );

void process_BLE_packet( void );

int needs_escape( char data );

extern volatile uint8_t g_hydra_binary_mode;
extern volatile uint8_t g_hydra_new_packet;

#endif /* BLU-XV1.0_H_ */