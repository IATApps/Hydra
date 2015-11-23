#pragma once

using namespace System;

#define		MAX_DATA_LENGTH		(16*4)

ref class SerialPacket
{
public:
	SerialPacket(void);

	property unsigned char Address
	{
		unsigned char get()
		{
			return _Address;
		}
		void set( unsigned char address )
		{
			_Address = address;
		}
	}

	property unsigned char PacketDescriptor
	{
		unsigned char get()
		{
			return _PacketDescriptor;
		}
		void set( unsigned char packetDescriptor )
		{
			_PacketDescriptor = packetDescriptor;
		}
	}

	property UInt16 Checksum
	{
		UInt16 get()
		{
			return _Checksum;
		}
		void set( UInt16 checksum )
		{
			_Checksum = checksum;
		}
	}

	property bool IsBatch
	{
		bool get()
		{
			if( _PacketDescriptor & 0x40 )
				return true;
			else
				return false;
		}
		void set( bool batch_enable )
		{
			if( batch_enable )
			{
				_PacketDescriptor |= 0x40;
			}
			else
			{
				_PacketDescriptor |= 0x40;
				_PacketDescriptor ^= 0x40;
			}
		}
	}

	property bool HasData
	{
		bool get()
		{
			if( _PacketDescriptor & 0x80 )
				return true;
			else
				return false;
		}
		void set( bool has_data )
		{
			if( has_data )
			{
				_PacketDescriptor |= 0x80;
			}
			else
			{
				_PacketDescriptor |= 0x80;
				_PacketDescriptor ^= 0x80;
			}
		}
	}

	property unsigned char BatchLength
	{
		unsigned char get()
		{
			return (_PacketDescriptor >> 2) & 0x0F;
		}
		void set( unsigned char length )
		{
			length &= 0x0F;
			// Clear batch length bits
			_PacketDescriptor |= (0x0F << 2);
			_PacketDescriptor ^= (0x0F << 2);
			// Set batch length bits
			_PacketDescriptor |= (length << 2);
		}
	}

	property unsigned char CommandFailed
	{
		unsigned char get()
		{
			return _PacketDescriptor & 0x01;
		}
		void set( unsigned char failed)
		{
			failed &= 0x01;
			_PacketDescriptor |= 0x01;
			_PacketDescriptor ^= 0x01;
			_PacketDescriptor |= failed;
		}
	}

	property unsigned char DataLength
	{
		unsigned char get()
		{
			if( HasData && IsBatch )
			{
				return 4*BatchLength;
			}
			if( HasData && !IsBatch )
			{
				return 4;
			}
			
			return 0;

		}
	}

	property unsigned char PacketLength
	{
		unsigned char get()
		{
			return DataLength + 7;
		}
	}

	void SetDataByte( int index, unsigned char value )
	{
		Data[index] = value;
	}
	unsigned char GetDataByte( int index ) { return Data[index]; };

	void ComputeChecksum( void )
	{
		UInt16 checksum;

		checksum = 0;

		checksum += (unsigned char)'s';
		checksum += (unsigned char)'n';
		checksum += (unsigned char)'p';
		checksum += _PacketDescriptor;
		checksum += _Address;

		for( int i = 0; i < DataLength; i++ )
		{
			checksum += Data[i];
		}

		_Checksum = checksum;
	}

private:
	unsigned char _Address;
	unsigned char _PacketDescriptor;
	UInt16 _Checksum;

	cli::array<unsigned char>^ Data;
};
