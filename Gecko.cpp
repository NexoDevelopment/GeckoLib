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

void Gecko::UploadMemory(uint32_t start_addr, uint32_t size, uint8_t* what)
{
	uint32_t data[] = { _byteswap_ulong(start_addr), _byteswap_ulong(start_addr + size) };
	array<unsigned char>^ arr = BitConverter::GetBytes(*(uint64_t*)&data[0]);

	this->stream->WriteByte(COMMAND_UPLOAD_MEMORY);
	this->stream->Write(arr, 0, 8);

	for (int i = 0; i < size; i++)
	{
		this->stream->WriteByte(what[i]);
	}

}

uint32_t Gecko::Get_Symbol(char* rplname, char* symname)
{
	array<unsigned char>^ send_buf = gcnew array<unsigned char>(1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1 + 1);
	array<unsigned char>^ recv_buf = gcnew array<unsigned char>(8);

	uint8_t* buf = (uint8_t*)malloc(1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1 + 1);
	uint32_t* buf2 = (uint32_t*)buf;

	buf[0] = 0x71;
	buf[1] = 8 + strlen(rplname) + 1 + strlen(symname) + 1;
	*(uint32_t*)&buf[2] = _byteswap_ulong(8);
	*(uint32_t*)&buf[6] = _byteswap_ulong(8 + strlen(rplname) + 1);

	memcpy(&buf[10], rplname, strlen(rplname) + 1);
	memcpy(&buf[10 + strlen(rplname) + 1], symname, strlen(symname) + 1);
	buf[1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1] = 0;

	for (int i = 0; i < (1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1 + 1); i++)
	{
		send_buf[i] = buf[i];
	}

	FILE* f = fopen("C:/Users/UserPC/Desktop/data.bin", "wb");
	fwrite(buf, 1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1 + 1, 1, f);
	fclose(f);

	this->stream->Write(send_buf, 0, 1 + 1 + 8 + strlen(rplname) + 1 + strlen(symname) + 1 + 1);

	this->stream->Read(recv_buf, 0, 4);

	uint32_t ret = ((recv_buf[0] << 24) + (recv_buf[1] << 16) + (recv_buf[2] << 8) + (recv_buf[3]));

	return ret;

}

uint32_t Gecko::RPC(RPC_Args* args)
{

	uint8_t* buf = (uint8_t*)args;

	array<unsigned char>^ dummy = gcnew array<unsigned char>(4);
	array<unsigned char>^ dummy2 = gcnew array<unsigned char>(4);

	this->stream->WriteByte(COMMAND_REMOTE_PROCEDURE_CALL);

	for (int i = 0; i < sizeof(RPC_Args); i++)
	{
		this->stream->WriteByte(buf[i]);
	}

	this->stream->Read(dummy, 0, 4);	// r3
	this->stream->Read(dummy2, 0, 4);	// r4

	uint32_t ret = ((dummy[0] << 24) + (dummy[1] << 16) + (dummy[2] << 8) + (dummy[3]));

	return ret;

}