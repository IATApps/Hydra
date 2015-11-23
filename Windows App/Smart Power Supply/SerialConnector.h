#pragma once

#include "SerialPacket.h"

using namespace System;
using namespace System::IO::Ports;
using namespace System::Windows::Forms;

#define		RX_BUFFER_SIZE		1000
#define		RX_PACKET_BUF_SIZE	20
#define		TX_PACKET_BUF_SIZE	20

#define		MAX_PACKET_BYTES	100

delegate void SerialPacketReceivedEventHandler( SerialPacket^ packet );
delegate void SerialPacketErrorEventHandler( String^ error );

ref class SerialConnector: public System::IO::Ports::SerialPort
{
public:
	SerialConnector(void);

	void TransmitPacket( SerialPacket^ packet );
	void TransmitRaw( System::String^ data );

	event SerialPacketReceivedEventHandler^ OnSerialPacketReceived;
	event SerialPacketErrorEventHandler^ OnSerialPacketError;
	
private:
	void SerialDataReceived( Object^ sender, SerialDataReceivedEventArgs^ e );
	cli::array<unsigned char,1>^ RXBuffer;
	int RXBufPtr;		// Points to the index in the serial buffer where the next item should be placed

	SerialPacket^ RXPacket;
	cli::array<SerialPacket^>^ TXPacketBuffer;
};
