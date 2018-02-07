#pragma once
#include <stack>
#include <map>
#include <sstream>
#include <ios>
#include <iomanip>

#define CHIP_8_MEMORY_SIZE 4096
#define INTERPRETER_SIZE 512
#define START_CHIP_8_PROGRAM INTERPRETER_SIZE
#define DISPLAY_REFRESH_SIZE 256
#define RESERVED_SPACE_SIZE 96
#define HIGHEST_PC_VALUE (CHIP_8_MEMORY_SIZE-DISPLAY_REFRESH_SIZE-RESERVED_SPACE_SIZE)
#define MAX_PROGRAM_SIZE (CHIP_8_MEMORY_SIZE-INTERPRETER_SIZE-DISPLAY_REFRESH_SIZE-RESERVED_SPACE_SIZE)

class Chip8
{
public:
	static Chip8* GetInstance();
	int LoadProgram(wchar_t *buffer, int size);
	unsigned short GetOpcode(unsigned short location);
	unsigned short GetProgramSize();
	void Reset();
	int ExecuteNextInstruction();
	int DecodeInstructionAt(unsigned short programCounter, std::wostringstream& description);
	void KeyPress(char key);
	unsigned long long GetDisplayRow(unsigned char row);
	~Chip8();
private:
	Chip8();

	static constexpr int FONT_HEIGHT = 5;
	static constexpr int FONT_WIDTH = 4;
	static constexpr int NUMBER_OF_FONTS = 16;
	static constexpr int DISPLAY_HEIGHT = 32;

	unsigned char *m_memory;
	int m_programSize;
	static Chip8* m_instance;

	unsigned char m_registers[16]; // These are the V registers but I hate naming them V
	unsigned short m_pc;
	unsigned short m_addressRegister; // This is the I memory register
	unsigned char m_delayTimer;
	unsigned char m_sleepTimer;
	std::stack<unsigned short> m_stack;
	std::map<char,int> m_validKeys;
	unsigned char m_keyPressed;
	std::wostringstream  m_scratch; 

	// The Chip-8 display is 64x32 pixels. Store as 32 colums of 64 bits (8 bytes)
	unsigned long long m_graphicsDisplay[DISPLAY_HEIGHT];

	int DecodeExecute(unsigned short& programCounter, bool decodeOnly, std::wostringstream& description);
	void ClearDisplay();
	void ProcessRegisterSet(unsigned char registerNum, unsigned char value);
	void ProcessRegisterAddition(unsigned char registerNum, unsigned char value);
	int ProcessBitRegisterOperation(unsigned char firstRegister, unsigned char secondRegister, unsigned char opSubType, std::wostringstream& description, bool decodeOnly);
	void ProcessAddressRegisterSet(unsigned short address);
	void ProcessDisplay(unsigned char firstRegister, unsigned char secondRegister, unsigned char height);
	int ProcessMemoryOperation(unsigned char registerNum, unsigned char constValue, std::wostringstream& description, bool decodeOnly);
	void ProcessFontOperation(unsigned char registerNum);	
	void ProcessBCDOperation(unsigned char registerNum);
	int ProcessFillFromRegisters(unsigned char registerNum);
	int ProcessFillRegisters(unsigned char registerNum);
	void ProcessRandom(unsigned char registerNum, unsigned char constValue);
};

