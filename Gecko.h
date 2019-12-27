#pragma once

#include <inttypes.h>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Runtime::InteropServices;

#define COMMAND_WRITE_8 0x01
#define COMMAND_WRITE_16 0x02
#define COMMAND_WRITE_32 0x03
#define COMMAND_READ_MEMORY 0x04
#define COMMAND_READ_MEMORY_KERNEL 0x05
#define COMMAND_VALIDATE_ADDRESS_RANGE 0x06
#define COMMAND_MEMORY_DISASSEMBLE 0x08
#define COMMAND_READ_MEMORY_COMPRESSED 0x09
#define COMMAND_KERNEL_WRITE 0x0B
#define COMMAND_KERNEL_READ 0x0C
#define COMMAND_TAKE_SCREEN_SHOT 0x0D
#define COMMAND_UPLOAD_MEMORY 0x41
#define COMMAND_SERVER_STATUS 0x50
#define COMMAND_GET_DATA_BUFFER_SIZE 0x51
#define COMMAND_READ_FILE 0x52
#define COMMAND_READ_DIRECTORY 0x53
#define COMMAND_REPLACE_FILE 0x54
#define COMMAND_GET_CODE_HANDLER_ADDRESS 0x55
#define COMMAND_READ_THREADS 0x56
#define COMMAND_ACCOUNT_IDENTIFIER 0x57
#define COMMAND_FOLLOW_POINTER 0x60
#define COMMAND_REMOTE_PROCEDURE_CALL 0x70
#define COMMAND_GET_SYMBOL 0x71
#define COMMAND_MEMORY_SEARCH_32 0x72
#define COMMAND_ADVANCED_MEMORY_SEARCH 0x73
#define COMMAND_EXECUTE_ASSEMBLY 0x81
#define COMMAND_PAUSE_CONSOLE 0x82
#define COMMAND_RESUME_CONSOLE 0x83
#define COMMAND_IS_CONSOLE_PAUSED 0x84
#define COMMAND_SERVER_VERSION 0x99
#define COMMAND_GET_OS_VERSION 0x9A
#define COMMAND_SET_DATA_BREAKPOINT 0xA0
#define COMMAND_SET_INSTRUCTION_BREAKPOINT 0xA2
#define COMMAND_TOGGLE_BREAKPOINT 0xA5
#define COMMAND_REMOVE_ALL_BREAKPOINTS 0xA6
#define COMMAND_POKE_REGISTERS 0xA7
#define COMMAND_GET_STACK_TRACE 0xA8
#define COMMAND_GET_ENTRY_POINT_ADDRESS 0xB1
#define COMMAND_RUN_KERNEL_COPY_SERVICE 0xCD
#define COMMAND_IOSU_HAX_READ_FILE 0xD0
#define COMMAND_GET_VERSION_HASH 0xE0
#define COMMAND_PERSIST_ASSEMBLY 0xE1
#define COMMAND_CLEAR_ASSEMBLY 0xE2

struct RPC_Args
{
	uint32_t function_addr;
	uint32_t gpr[8]; // r3 to r11

};

public ref class Gecko
{
public:
	Gecko(String ^IP_Addr, int port);
	~Gecko();

	void Kill();
	void pokemem(uint32_t addr, uint32_t value);
	uint8_t *readmem(uint32_t addr, uint32_t size);
	void UploadMemory(uint32_t start_addr, uint32_t size, uint8_t* data);
	uint32_t Get_Symbol(char* rplname, char* symname);
	uint32_t RPC(RPC_Args* args);

	TcpClient^ tcp;
	NetworkStream^ stream;

private:

};
