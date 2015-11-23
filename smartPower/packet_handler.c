/*
 * packet_handler.c
 *
 * Created: 2/3/2013 6:51:55 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "config.h"
#include "packet_handler.h"
#include "USART.h"
#include "util.h"

/*******************************************************************************
* Function Name  : ProcessPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Handles packets received over the UART.  First, the packet is checked to make
sure it is understood.  If not, a packet is transmitted over the USART to that
effect.  If it is, then the following takes place:

1. If the packet is a write operation, then the data contained in data section
of the packet is copied to the relevant registers.
2. If the packet is a read operation, the relevant registers are copied into
a new packet.  The new packet is then transmitted.
3. Finally, the packet is sent to the DispatchPacket(.) function.  The DispatchPacket
function is used to handle packet-specific actions that must be performed when the
packet is received.  For example, a packet that alters the broadcast rate of the
device must change the broadcast timer configuration to reflect the change.  This
is handled by the DispatchPacket function.
						 
*******************************************************************************/
void process_packet( Packet* new_packet )
{
	 uint8_t address_valid = 1;
	 uint8_t batch_good = 1;
	 	 
	 // Check to make sure that the packet address is recognizable.  For example, if it involves
	 // a read or a write to the configuration array, then the address must be less than CONFIG_ARRAY_SIZE,
	 // which is defined in config.h.  Similarly, a read or write to the data array must be to an
	 // address lower than DATA_ARRAY_SIZE, and a command packet must have an address lower than COMMAND_COUNT
	 // (also defined in config.h)
	 
	 // Start with a check for configuration register reads/writes.  Note that valid configuration registers have addresses
	 // starting at CONFIG_REG_START_ADDRESS and ending at DATA_REG_START_ADDRESS - 1 (this is based on how the communication
	 // protocol is defined)
	 if( new_packet->address_type == ADDRESS_TYPE_CONFIG )
	 {
		  if( (new_packet->address-CONFIG_REG_START_ADDRESS) >= CONFIG_ARRAY_SIZE )
		  {
				address_valid = 0;
		  }
		  
		  // Check if this is a batch operation and, if so, whether it is valid (cannot allow a batch operation to go beyond array bounds)
		  if( new_packet->type & PACKET_IS_BATCH )
		  {
				if( (new_packet->address - CONFIG_REG_START_ADDRESS + ((new_packet->type >> 2) & PACKET_BATCH_LENGTH_MASK) - 1) >= CONFIG_ARRAY_SIZE )
				{
					 batch_good = 0;
				}
		  }
	 }
	 // Check for invalid data register addresses now...
	 else if( new_packet->address_type == ADDRESS_TYPE_DATA )
	 {
		  if( (new_packet->address-DATA_REG_START_ADDRESS) >= DATA_ARRAY_SIZE )
		  {
				address_valid = 0;
		  }
		  
		  // Check if this is a batch operation and, if so, whether it is valid (cannot allow a batch operation to go beyond array bounds)
		  if( new_packet->type & PACKET_IS_BATCH )
		  {
				if( (new_packet->address - DATA_REG_START_ADDRESS + ((new_packet->type >> 2) & PACKET_BATCH_LENGTH_MASK) - 1) >= DATA_ARRAY_SIZE )
				{
					 batch_good = 0;
				}
		  }
	 }
	 // Now check for invalid commands
	 else if( new_packet->address_type == ADDRESS_TYPE_COMMAND )
	 {
		  if( (new_packet->address - COMMAND_START_ADDRESS) >= COMMAND_COUNT )
		  {
				address_valid = 0;
		  }
	 }
	 
	 // If the address check failed, send a packet informing about the problem
	 if( !address_valid )
	 {
		  Packet response_packet;
		  
		  // Send "unknown address" packet
		  response_packet.type = PACKET_NO_DATA;
		  response_packet.address = COM_UNKNOWN_ADDRESS;
		  response_packet.data_length = 0;	// No data bytes
		  response_packet.checksum = compute_checksum( &response_packet );
		  
		  transmit_packet( &response_packet );
		  
		  return;
	 }

	 // If the the batch size was too large to be valid, send a packet informing about the problem
	 if( !batch_good )
	 {
		  Packet response_packet;
		  
		  // Send "invalid batch size" packet
		  response_packet.type = PACKET_NO_DATA;
		  response_packet.address = COM_INVALID_BATCH_SIZE;
		  response_packet.data_length = 0;	// No data bytes
		  response_packet.checksum = compute_checksum( &response_packet );
		  
		  transmit_packet( &response_packet );
		  
		  return;
	 }
	 
	 // Now process the packet.  
	 // Only read and write packets will be processed in this function.  Commands will be handled later, in the call to DispatchPacket(.)
	 if( new_packet->address_type != ADDRESS_TYPE_COMMAND )
	 {
		  // If the packet is performing a write operation, copy the data to the specified address(es)
		  // If it is a batch write operation, then multiple registers will be written.
		  if( new_packet->type & PACKET_HAS_DATA )
		  {
				int i;
				int address_offset = 0;
				
				// Received packets will always contain data in multiples of 4 bytes
				for( i = 0; i < new_packet->data_length; i += 4 )
				{
					 int address;
					 
					 if( new_packet->address_type == ADDRESS_TYPE_CONFIG )
					 {
						  address = new_packet->address - CONFIG_REG_START_ADDRESS + address_offset;
						  g_config.r[address] = ((uint32_t)new_packet->data[i+3]) | ((uint32_t)new_packet->data[i+2] << 8) | ((uint32_t)new_packet->data[i+1] << 16) | ((uint32_t)new_packet->data[i] << 24);
					 }
					 else
					 {
						  address = new_packet->address - DATA_REG_START_ADDRESS + address_offset;
						  g_data.r[address] = ((uint32_t)new_packet->data[i+3]) | ((uint32_t)new_packet->data[i+2] << 8) | ((uint32_t)new_packet->data[i+1] << 16) | ((uint32_t)new_packet->data[i] << 24);
					 }				 
					 
					 address_offset++;
				}
		  }
		  // If the packet is performing a read operation, construct a new packet containing the data to be transmitted
		  else
		  {
				send_global_data( new_packet->address, new_packet->address_type, (new_packet->type & PACKET_IS_BATCH), (new_packet->type >> 2) & PACKET_BATCH_LENGTH_MASK );
		  }
	 }
	 
	 // Now dispatch the packet for additional processing
	 dispatch_packet( new_packet );
}

/*******************************************************************************
* Function Name  : SendGlobalData
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Constructs a packet containing the specified data and sends it over the USART
						 
*******************************************************************************/
void send_global_data(uint8_t address, uint8_t address_type, uint8_t packet_is_batch, uint8_t batch_size)
{
	 Packet response_packet;
	 int i;
	 
	 response_packet.type = PACKET_HAS_DATA;
	 response_packet.address = address;
	 
	 // If this is a batch read, then define the packet accordingly
	 if( packet_is_batch )
	 {
		  response_packet.type |= PACKET_IS_BATCH;
		  response_packet.type |= (batch_size << 2);
		  response_packet.data_length = 4*batch_size;
		  
		  for( i = 0; i < batch_size; i++ )
		  {
				// Check to determine whether this is a configuration register access or a data register access
				if( address_type == ADDRESS_TYPE_CONFIG )
				{
					 response_packet.data[4*i] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS + i] >> 24) & 0x0FF);
					 response_packet.data[4*i+1] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS + i] >> 16) & 0x0FF);
					 response_packet.data[4*i+2] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS + i] >> 8) & 0x0FF);
					 response_packet.data[4*i+3] = (uint8_t)(g_config.r[address - CONFIG_REG_START_ADDRESS + i] & 0x0FF);
				}
				else
				{
					 response_packet.data[4*i] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS + i] >> 24) & 0x0FF);
					 response_packet.data[4*i+1] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS + i] >> 16) & 0x0FF);
					 response_packet.data[4*i+2] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS + i] >> 8) & 0x0FF);
					 response_packet.data[4*i+3] = (uint8_t)(g_data.r[address - DATA_REG_START_ADDRESS + i] & 0x0FF);
				}
										
		  }
	 }
	 // If this is not a batch read, just transmit the data found in the specified address
	 else
	 {
		  response_packet.data_length = 4;
		  
		  // Check to determine whether this is a configuration register access or a data register access
		  if( address_type == ADDRESS_TYPE_CONFIG )
		  {
				response_packet.data[0] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS] >> 24) & 0x0FF);
				response_packet.data[1] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS] >> 16) & 0x0FF);
				response_packet.data[2] = (uint8_t)((g_config.r[address - CONFIG_REG_START_ADDRESS] >> 8) & 0x0FF);
				response_packet.data[3] = (uint8_t)(g_config.r[address - CONFIG_REG_START_ADDRESS] & 0x0FF);
		  }
		  else
		  {
				response_packet.data[0] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS] >> 24) & 0x0FF);
				response_packet.data[1] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS] >> 16) & 0x0FF);
				response_packet.data[2] = (uint8_t)((g_data.r[address - DATA_REG_START_ADDRESS] >> 8) & 0x0FF);
				response_packet.data[3] = (uint8_t)(g_data.r[address - DATA_REG_START_ADDRESS] & 0x0FF);
		  }
	 }
	 
	 // The response packet should now be filled with data.  Compute the Checksum and transmit the packet
	 response_packet.checksum = compute_checksum( &response_packet );

	 transmit_packet( &response_packet );
	 
}

/*******************************************************************************
* Function Name  : DispatchPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Handles processing specific to individual packets.   This function is called
by ProcessPacket after all requisite read and write operation are performed.
						 
*******************************************************************************/
void dispatch_packet( Packet* new_packet )
{
	 Packet response_packet;
	 
	 // If this packet wrote new data to the device, copy that data to global structures for convenience
	 if( new_packet->type & PACKET_HAS_DATA )
	 {
		 // Copy to local variables for convenience.  The new settings have already been written to the configuration array
		 copy_config_to_local();
		 // Set the power supply outputs based on the new settings
		 update_outputs_from_config();
		 // Save the new configuration to the EEPROM		 
		 save_config_to_ROM();
	 }
	 
	 switch(new_packet->address)
	 {
		case GET_FW_VERSION:
			response_packet.type = PACKET_HAS_DATA;
			response_packet.address = GET_FW_VERSION;
			response_packet.data_length = 4;
			response_packet.data[0] = (uint8_t)'H';
			response_packet.data[1] = (uint8_t)'P';
			response_packet.data[2] = (uint8_t)'0';
			response_packet.data[3] = (uint8_t)'A';
			
			response_packet.checksum = compute_checksum( &response_packet );
			
			transmit_packet( &response_packet );
		break;
		
		case WRITE_TO_FLASH:
			save_config_to_ROM();
			
			send_command_success_packet( new_packet->address );
		break;
		
		default:
			// If this was a write command, send a "success" packet to signal that the write was successful
			// A read packet does not need this because the returned data packet will fill the role.  Similarly,
			// a command packet does not need it because commands are handled above
			if( new_packet->type & PACKET_HAS_DATA )
			{
				send_command_success_packet( new_packet->address );
			}
		break;
		  
	 }
	 
}

/*******************************************************************************
* Function Name  : SendCommandSuccessPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Sends a packet signifying that the command with address 'address' was completed
properly.
						 
*******************************************************************************/
void send_command_success_packet( uint8_t address )
{
	 Packet packet;
	 
	 packet.type = PACKET_NO_DATA;
	 packet.address = address;
	 packet.data_length = 0;
	 packet.checksum = compute_checksum( &packet );
	 
	 transmit_packet( &packet );
}

/*******************************************************************************
* Function Name  : SendCommandFailedPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Sends a packet signifying that the command with address 'address' was failed
to complete properly.
						 
*******************************************************************************/
void send_command_failed_packet( uint8_t address )
{
	 Packet packet;
	 
	 packet.type = PACKET_NO_DATA | PACKET_COMMAND_FAILED;
	 packet.address = address;
	 packet.data_length = 0;
	 packet.checksum = compute_checksum( &packet );
	 
	 transmit_packet( &packet );
}

/*******************************************************************************
* Function Name  : SendStatusPacket
* Input          : None
* Output         : None
* Return         : None
* Description    : 

Sends a packet containing the contents of the status register
						 
*******************************************************************************/
void SendStatusPacket()
{
	
}
