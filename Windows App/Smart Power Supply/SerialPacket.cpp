#include "StdAfx.h"
#include "SerialPacket.h"

SerialPacket::SerialPacket(void)
{
	this->HasData = false;
	this->IsBatch = false;
	this->Address = 0;
	this->BatchLength = 0;
	this->Checksum = 0;
	this->CommandFailed = 0;

	this->Data = gcnew cli::array<unsigned char>(MAX_DATA_LENGTH);
}
