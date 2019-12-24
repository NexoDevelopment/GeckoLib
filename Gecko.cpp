#include "Gecko.h"
#include <stdio.h>
#include <string.h>
#include <intrin.h>
#include <malloc.h>

Gecko::Gecko(String^ IP_Addr, int port)
{

	this->tcp = gcnew TcpClient(IP_Addr, port);
	this->stream = this->tcp->GetStream();

	printf("Connected to %s:%d\n", IP_Addr, port);


}

Gecko::~Gecko()
{

	this->stream->Close();
	this->tcp->Close();

}

void Gecko::Kill()
{
	this->stream->Close();
	this->tcp->Close();
}

void Gecko::pokemem(uint32_t addr, uint32_t value)
{

	uint32_t data[] = { _byteswap_ulong(addr), _byteswap_ulong(value) };
	array<unsigned char> ^arr = BitConverter::GetBytes(*(uint64_t*)&data[0]);

	this->stream->WriteByte(COMMAND_WRITE_32);
	this->stream->Write(arr, 0, 8);

}

uint8_t* Gecko::readmem(uint32_t addr, uint32_t size)
{

	uint32_t addr_init = addr;
	uint8_t* buffer = (uint8_t*)malloc(size);

	if (size > 0x4F00)
	{
		for (int i = 0; i < (size/0x4F00); i++)
		{
			uint32_t data[] = { _byteswap_ulong(addr), _byteswap_ulong(addr + 0x4F00) };
			array<unsigned char>^ arr = BitConverter::GetBytes(*(uint64_t*)&data[0]);
			array<unsigned char>^ recv_buf = gcnew array<unsigned char>(0x4F00);

			this->stream->WriteByte(COMMAND_READ_MEMORY);
			this->stream->Write(arr, 0, 8);
			
			uint8_t rc = this->stream->ReadByte();
			if (rc == 0xb0)
			{
				memset(buffer + (addr - addr_init), 0, 0x4F00);
			}
			else if (rc == 0xbd)
			{
				for (int i = 0; i < 0x4F00; i++)
				{
					buffer[i + (addr - addr_init)] = this->stream->ReadByte();
				}
			}

			delete recv_buf;
			delete arr;

			size -= 0x4F00;
			addr += 0x4F00;
		}

		if (size != 0)
		{
			uint32_t data[] = { _byteswap_ulong(addr), _byteswap_ulong(addr + size) };
			array<unsigned char>^ arr = BitConverter::GetBytes(*(uint64_t*)&data[0]);
			array<unsigned char>^ recv_buf = gcnew array<unsigned char>(size);

			this->stream->WriteByte(COMMAND_READ_MEMORY);
			this->stream->Write(arr, 0, 8);

			uint8_t rc = this->stream->ReadByte();
			if (rc == 0xb0)
			{
				memset(buffer + (addr - addr_init), 0, size);
			}
			else if (rc == 0xbd)
			{
				for (int i = 0; i < size; i++)
				{
					buffer[i + (addr - addr_init)] = this->stream->ReadByte();
				}
			}

			delete recv_buf;
			delete arr;
		}
	}
	else
	{

		uint32_t data[] = { _byteswap_ulong(addr), _byteswap_ulong(addr + size) };
		array<unsigned char>^ arr = BitConverter::GetBytes(*(uint64_t*)&data[0]);
		array<unsigned char>^ recv_buf = gcnew array<unsigned char>(size);

		this->stream->WriteByte(COMMAND_READ_MEMORY);
		this->stream->Write(arr, 0, 8);

		uint8_t rc = this->stream->ReadByte();
		if (rc == 0xb0)
		{
			memset(buffer, 0, size);
		}
		else if (rc == 0xbd)
		{
			for (int i = 0; i < size; i++)
			{
				buffer[i] = this->stream->ReadByte();
			}
		}

		delete recv_buf;
		delete arr;

	}

	return buffer;

}