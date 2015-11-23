#include "StdAfx.h"
#include "SerialConnector.h"

SerialConnector::SerialConnector(void)
{
	// Create RX buffer
	RXBuffer = gcnew cli::array<unsigned char,1>(RX_BUFFER_SIZE);
	RXBufPtr = 0;

	RXPacket = gcnew SerialPacket();
	TXPacketBuffer = gcnew cli::array<SerialPacket^>(TX_PACKET_BUF_SIZE);

	// Add event handler to receive serial data
	this->DataReceived += gcnew SerialDataReceivedEventHandler(this, &SerialConnector::SerialDataReceived);
}

void SerialConnector::SerialDataReceived( Object^ sender, SerialDataReceivedEventArgs^ e )
{
	int BytesReturned;
	bool found_packet;
	int packet_start_index;
	int packet_index;

	try
	{
		int bytes_to_read = this->BytesToRead;

		if( bytes_to_read + RXBufPtr >= RX_BUFFER_SIZE )
		{
			bytes_to_read = RX_BUFFER_SIZE - 1 - RXBufPtr;
		}

		BytesReturned = this->Read( RXBuffer, RXBufPtr, bytes_to_read );
	}
	catch( Exception^ e )
	{
		OnSerialPacketError(L"ERROR: FAILED TO READ SERIAL DATA. Exception: " + e->Message );

		return;
	}

	RXBufPtr += BytesReturned;

	// If there are enough bytes in the buffer to construct a full packet, then check data.
    // There are RXbufPtr bytes in the buffer at any given time
    while (RXBufPtr >= 7)
    {
        // Search for the packet start sequence
        found_packet = false;
        packet_start_index = 0;
        for (packet_index = 0; packet_index < (RXBufPtr - 2); packet_index++)
        {
            if (RXBuffer[packet_index] == 's' && RXBuffer[packet_index + 1] == 'n' && RXBuffer[packet_index + 2] == 'p')
            {
                found_packet = true;
                packet_start_index = packet_index;

                break;
            }
        }

		// If packet start sequence was not found, then remove all but the last three bytes from the buffer.  This prevents
		// bad data from filling the buffer up.
		if( !found_packet )
		{
			RXBuffer[0] = RXBuffer[RXBufPtr-3];
			RXBuffer[1] = RXBuffer[RXBufPtr-2];
			RXBuffer[2] = RXBuffer[RXBufPtr-1];

			RXBufPtr = 3;
		}

		// If a packet start sequence was found, extract the packet descriptor byte.
		// Make sure that there are enough bytes in the buffer to consitute a full packet
		int indexed_buffer_length = RXBufPtr - packet_start_index;
		if (found_packet && (indexed_buffer_length >= 7))
		{
			unsigned char packet_descriptor = (UInt32)RXBuffer[packet_start_index + 3];
			unsigned char address = (UInt32)RXBuffer[packet_start_index + 4];

			// Check the R/W bit in the packet descriptor.  If it is set, then this packet does not contain data 
			// (the packet is either reporting that the last write operation was succesfull, or it is reporting that
			// a command finished).
			// If the R/W bit is cleared and the batch bit B is cleared, then the packet is 11 bytes long.  Make sure
			// that the buffer contains enough data to hold a complete packet.
			bool HasData;
			bool IsBatch;
			unsigned char BatchLength;

			int packet_length;

			if( ( packet_descriptor & 0x80 ) )
			{
				HasData = true;
			}
			else
			{
				HasData = false;
			}

			if( ( packet_descriptor & 0x40 ) )
			{
				IsBatch = true;
			}
			else
			{
				IsBatch = false;
			}
			
			if( HasData && !IsBatch )
			{
				packet_length = 11;
			}
			else if( HasData && IsBatch )
			{
				// If this is a batch operation, then the packet length is: length = 5 + 4*L + 2, where L is the length of the batch.
				// Make sure that the buffer contains enough data to parse this packet.
				BatchLength = (packet_descriptor >> 2) & 0x0F;
				packet_length = 5 + 4*BatchLength + 2;				
			}
			else if( !HasData )
			{
				packet_length = 7;
			}

			if( indexed_buffer_length < packet_length )
			{
				return;
			}

			SerialPacket^ NewPacket = gcnew SerialPacket();

			// If code reaches this point, then there enough bytes in the RX buffer to form a complete packet.
			NewPacket->Address = address;
			NewPacket->PacketDescriptor = packet_descriptor;

			// Copy data bytes into packet data array
			int data_start_index = packet_start_index + 5;
			for( int i = 0; i < NewPacket->DataLength; i++ )
			{
				NewPacket->SetDataByte( i, RXBuffer[data_start_index + i] );
			}

			// Now record the checksum
			UInt16 Checksum = ((UInt16)RXBuffer[packet_start_index + packet_length - 2] << 8) | ((UInt16)RXBuffer[packet_start_index + packet_length - 1]);

			// Compute the checksum and compare with the one given in the packet.  If different, ignore this packet
			NewPacket->ComputeChecksum();

			if( Checksum == NewPacket->Checksum )
			{
				OnSerialPacketReceived( NewPacket );
			}
			else
			{
				OnSerialPacketError(L"Received packet with bad checksum.  Packet discarded.");
			}

			// At this point, the newest packet has been parsed and copied into the RXPacketBuffer array.
			// Copy all received bytes that weren't part of this packet into the beginning of the
            // buffer.  Then, reset RXbufPtr.
            for (int index = 0; index < (RXBufPtr - (packet_start_index + packet_length)); index++)
            {
                RXBuffer[index] = RXBuffer[(packet_start_index + packet_length) + index];
            }

			RXBufPtr -= (packet_start_index + packet_length);
		}
		else
		{
			return;
		}
	
	}
}

void SerialConnector::TransmitPacket( SerialPacket^ packet )
{
	cli::array<unsigned char,1>^ data_array = gcnew cli::array<unsigned char,1>(MAX_PACKET_BYTES);

	// Fill data_array with the actual packet contents and send via the serial port
	data_array[0] = 's';
	data_array[1] = 'n';
	data_array[2] = 'p';
	data_array[3] = packet->PacketDescriptor;
	data_array[4] = packet->Address;

	try
	{
		for( int i = 0; i < packet->DataLength; i++ )
		{
			data_array[5+i] = packet->GetDataByte(i);
		}

		packet->ComputeChecksum();

		UInt16 checksum = packet->Checksum;

		data_array[5 + packet->DataLength] = (checksum >> 8);
		data_array[5 + packet->DataLength + 1] = (checksum & 0x0FF);

		// Transmit data
		this->Write(data_array,0,packet->PacketLength);
	}
	catch( Exception^ /*e*/ )
	{
		OnSerialPacketError(L"FAILED TO TRANSMIT PACKET.  Is the COM port connnected?" );
	}
}

// Function call transmits the null-terminated character string pointed to by System::String^ data
void SerialConnector::TransmitRaw( System::String^ data )
{
	this->Write(data);
}
