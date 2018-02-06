#include "stdafx.h"
#include "Chip8.h"
#include <cstdlib>
#include <ctime>

Chip8 *Chip8::m_instance = 0;

Chip8::Chip8() :
	m_programSize(0),
	m_validKeys({	{'1', 0x1},
					{'2', 0x2},
					{'3', 0x3},
					{'4', 0xC},
					{'Q', 0x4},
					{'W', 0x5},
					{'E', 0x6},
					{'R', 0xD},
					{'A', 0x7},
					{'S', 0x8},
					{'D', 0x9},
					{'F', 0xE},
					{'Z', 0xA},
					{'X', 0x0},
					{'C', 0xB},
					{'V', 0xF } }),
	m_keyPressed(0xF0) // Only the range 0x0 to 0xF is valid so set to some arbitrary invalid number

{
	m_memory = new unsigned char[CHIP_8_MEMORY_SIZE];

	// The fonts are 4 bits wide so they are stored in the upper nibble of a byte
	static const uint8_t fonts[NUMBER_OF_FONTS * FONT_HEIGHT] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80, // F
	};

	// Most modern CHIP-8 use the space reserved for the CHIP-8 interpreter to store the fonts
	// since it would otherwise be unused
	for (int x = 0; x < NUMBER_OF_FONTS * FONT_HEIGHT; x++)
	{
		m_memory[x] = fonts[x];
	}

}


Chip8::~Chip8()
{
	delete[] m_memory;
}

Chip8* Chip8::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new Chip8;
	}
	return m_instance;
}

/*****************************************************************************************************************************************/
// 
// LoadProgram - Loads the chip rom into memory
//
// Inputs - buffer (wide character buffer holding the contents of the rom
//          size (integer size of the buffer)
//
// Outputs - None
//
// Notes - This is a little bit ugly but I wanted to be able to open files that may make use of the wide character. However, that means
//         that you can only read in wchar_t data which is sign extends the binary data from the file. This strips off the extra byte
//         which is 0x00 and restores it to the binary data
/*****************************************************************************************************************************************/
int Chip8::LoadProgram(wchar_t *buffer, int size)
{
	unsigned char *currentMemoryAddr = &(m_memory[INTERPRETER_SIZE]); // The program starts after the memory reserved for the interpreter

	if (0 != size % 2)
	{
		return -1;
	}

	for (int x = 0; x < size; x++)
	{
		*(currentMemoryAddr++) = (unsigned char)*(buffer++);
	}
	m_programSize = size;

	return 0;
}

unsigned short Chip8::GetOpcode(unsigned short location)
{
	unsigned short returnVal = 0;
	// Opcodes are 2 bytes, so opcode0 is at offset 0, opcode1 is at offset 2 and opcodeX is at offset X*2
	if (location < HIGHEST_PC_VALUE )
	{
		returnVal = m_memory[location] << 8;
		returnVal |= m_memory[location +1];
	}
	else
	{
		returnVal = 0xFFFF;
	}

	return returnVal;
}

unsigned short Chip8::GetProgramSize()
{
	return m_programSize;
}

int Chip8::ExecuteNextInstruction()
{
	int returnValue = 0;
	unsigned short opcode = GetOpcode(m_pc);
	unsigned char  operationType = (opcode & 0xF000) >> 12;
	// After the first nibble, the rest depends on the operation type, but get all possible combinations so we don't have to do them later
	unsigned char firstRegister = (opcode & 0x0F00) >> 8;
	unsigned char secondRegister = (opcode & 0x00F0) >> 4;
	unsigned char opSubType = (opcode & 0x000F);
	unsigned char constValue = (opcode & 0x00FF);
	unsigned short address = (opcode & 0x0FFF);
	bool flowControl = false; // This opcode is a flow control operation either calling or returning from the PC

	if (0 != m_delayTimer)
		m_delayTimer--;
	if (0 != m_sleepTimer)
		m_sleepTimer--;
	else
		returnValue |= 0x4;

	switch (operationType)
	{
		case 0x00:
			// I'm ignore the case of 0NNN which doesn't seem to be valid for modern interpreters
			if ((0x0 != firstRegister) || (0xE != secondRegister))
			{
				returnValue = 0x1;
			}
			else
			{
				if (0 == opSubType)
				{
					ClearDisplay();
				}
				else if (0xE == opSubType)
				{
					m_pc = m_stack.top();
					m_stack.pop();
					flowControl = true;
				}
				else
				{
					returnValue |= 0x1;
				}
			}
			break;
		case 0x1:
			if ((address > START_CHIP_8_PROGRAM) && (address < (START_CHIP_8_PROGRAM + m_programSize)))
			{
				m_pc = address;
				flowControl = true;
			}
			break;
		case 0x02:
			if ((address > START_CHIP_8_PROGRAM) && (address < (START_CHIP_8_PROGRAM + m_programSize)))
			{
				m_stack.push(m_pc+2); // Move to the next instruction passed the subroutine call
				m_pc = address;
				flowControl = true;
			}
			else
			{
				returnValue |= 0x1; // Set a bad opcode if address is out of range
			}
			break;
		case 0x03:
			if (m_registers[firstRegister] == constValue)
			{
				m_pc += 2; // We'll add 2 more at the end of the case skipping the next instruction
			}
			break;
		case 0x04:
			if (m_registers[firstRegister] != constValue)
			{
				m_pc += 2; // We'll add 2 more at the end of the case skipping the next instruction
			}
			break;
		case 0x06:
			ProcessRegisterSet(firstRegister, constValue);
			break;
		case 0x07:
			ProcessRegisterAddition(firstRegister, constValue);
			break;
		case 0x08:
			returnValue |= ProcessBitRegisterOperation(firstRegister, secondRegister, opSubType);
			break;
		case 0x0A:
			ProcessAddressRegisterSet(address);
			break;
		case 0x0C:
			ProcessRandom(firstRegister, constValue);
			break;
		case 0x0D:
			ProcessDisplay(firstRegister, secondRegister, opSubType);
			returnValue |= 0x2;
			break;
		case 0x0F:
			returnValue = ProcessMemoryOperation(firstRegister, constValue);
			break;
		case 0x0E:
			if (0x9E == constValue)
			{
				if (m_keyPressed == m_registers[firstRegister])
				{
					m_pc += 2;
					m_keyPressed = 0xF0;
				}
			}
			else if (0xA1 == constValue)
			{
				if (m_keyPressed != m_registers[firstRegister])
				{
					m_pc += 2;
					m_keyPressed = 0xF0;
				}
			}
			else
			{
				returnValue |= 1;
			}
			break;
		default:
			// handle invalid operations later
			returnValue |= 0x1;
			break;
	}
	
	if (!flowControl) m_pc += 2;
	if (returnValue & 0x1)
		m_registers[0] = 13;
	return returnValue;
}

void Chip8::ProcessRegisterSet(unsigned char registerNum, unsigned char value)
{
	m_registers[registerNum] = value;
}

void Chip8::ProcessRegisterAddition(unsigned char registerNum, unsigned char value)
{
	m_registers[registerNum] = m_registers[registerNum] + value;
}

int Chip8::ProcessBitRegisterOperation(unsigned char firstRegister, unsigned char secondRegister, unsigned char opSubType)
{
	int returnValue = 0;
	switch(opSubType)
	{
		case 0x00:
			m_registers[firstRegister] = m_registers[secondRegister];
			break;
		case 0x01:
			m_registers[firstRegister] |= m_registers[secondRegister];
			break;
		case 0x02:
			m_registers[firstRegister] &= m_registers[secondRegister];
			break;
		case 0x03:
			m_registers[firstRegister] ^= m_registers[secondRegister];
			break;
		case 0x04:
		{
			unsigned short value = (unsigned short)m_registers[firstRegister] + m_registers[secondRegister];
			if (value > 0xFF)
				m_registers[15] = 1;
			else
				m_registers[15] = 0;
			m_registers[firstRegister] = (unsigned char)value;
		}
			break;
		case 0x05:
			if (m_registers[secondRegister] > m_registers[firstRegister])
				m_registers[15] = 0;
			else
				m_registers[15] = 1;
			m_registers[firstRegister] -= m_registers[secondRegister];
			break;
		case 0x06:
			m_registers[15] = m_registers[secondRegister] & 0x01;
			m_registers[secondRegister] >>= 1;
			m_registers[firstRegister] = m_registers[secondRegister];
			break;
		case 0x07:
			if (m_registers[firstRegister] > m_registers[secondRegister])
				m_registers[15] = 0;
			else
				m_registers[15] = 1;
			m_registers[firstRegister] = m_registers[secondRegister] - m_registers[firstRegister];
			break;
		case 0x0E:
			m_registers[15] = m_registers[secondRegister] & 0x80 >> 15;
			m_registers[secondRegister] <<= 1;
			m_registers[firstRegister] = m_registers[secondRegister];
			break;
		default:
			returnValue = 0x01;
			break;
	}
	return returnValue;
}


void Chip8::ProcessAddressRegisterSet(unsigned short address)
{
	m_addressRegister = address;
}

void Chip8::ProcessRandom(unsigned char firstRegister, unsigned char constValue)
{
	std::srand((unsigned int)std::time(nullptr)); // use current time as seed for random generator
	unsigned char random_variable = std::rand() % 255;
	m_registers[firstRegister] = random_variable & constValue;
}

void Chip8::ProcessDisplay(unsigned char firstRegister, unsigned char secondRegister, unsigned char height)
{
	unsigned char xCoor = m_registers[firstRegister];
	unsigned char yCoor = m_registers[secondRegister];
	bool collision = false;

	if (1 == height)
	{
		int yy = 32;
	}
	for (int x = 0; x < height; x++)
	{
		unsigned char row = (yCoor + x) % DISPLAY_HEIGHT;
		unsigned char previousSprite = (m_graphicsDisplay[row] >> xCoor) & 0xFF;

		// We need to reverse the order of the bits as it appears that a pattern of 0x80 means that the first bit in the sprite should be set
		unsigned char thisSprite = (unsigned char) (((m_memory[m_addressRegister + x] * 0x0802LU & 0x22110LU) | (m_memory[m_addressRegister + x]
			                        * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
		m_graphicsDisplay[row] ^= ((unsigned long long) thisSprite << xCoor);
		unsigned char changeMask = 0x80;
		unsigned char change = thisSprite ^ previousSprite;
		if (!collision) // No need to do this if we have already registered a collision
		{
			while (0 != changeMask)
			{
				if ((0 == (change & changeMask)) && (1 == (previousSprite & changeMask)))
				{
					collision = true;
					break;
				}
				changeMask >>= 1;
			}
		}

	}

	m_registers[15] = (collision ? 1 : 0);
}

unsigned long long Chip8::GetDisplayRow(unsigned char row)
{
	if (row < 32)
		return m_graphicsDisplay[row];
	else
		return 0;
}

int Chip8::ProcessMemoryOperation(unsigned char registerNum, unsigned char constValue)
{
	int returnValue = 0;
	switch (constValue)
	{
		case 0x07:
			m_registers[registerNum] = m_delayTimer;
			break;
		case 0x15:
			m_delayTimer = m_registers[registerNum];
			break;
		case 0x18:
			m_sleepTimer = m_registers[registerNum];
			break;
		case 0x29:
			ProcessFontOperation(registerNum);
			break;
		case 0x33:
			ProcessBCDOperation(registerNum);
			break;
		case 0x65:
			returnValue = ProcessFillRegisters(registerNum);
			break;
		default:
			returnValue = 0x1;
	}
	return returnValue;
}

void Chip8::ProcessFontOperation(unsigned char registerNum)
{
	unsigned short offset = m_registers[registerNum] * FONT_HEIGHT;
	m_addressRegister = offset;
}

void Chip8::ProcessBCDOperation(unsigned char registerNum)
{
	unsigned char value = m_registers[registerNum];

	m_memory[m_addressRegister] = value / 100;
	m_memory[m_addressRegister+1] = (value / 10) % 10;
	m_memory[m_addressRegister+2] = (value % 100) % 10;
}

int Chip8::ProcessFillRegisters(unsigned char registerNum)
{
	if ((m_addressRegister + registerNum) >= CHIP_8_MEMORY_SIZE)
		return 0x1;

	for (int x = 0; x <= registerNum; x++)
	{
		m_registers[x] = m_memory[m_addressRegister++];
	}

	return 0;
}

void Chip8::Reset()
{
	for (int x = 0; x < 16; x++)
		m_registers[x] = 0;
	m_pc = 0x200; // Start at the beginning
	m_delayTimer = 0;
	m_sleepTimer = 0;
	while (!m_stack.empty())
	{
		m_stack.pop();
	}
	ClearDisplay();
}

void Chip8::ClearDisplay()
{
	for (int x = 0; x < 32; x++)
		m_graphicsDisplay[x] = 0;
}

void Chip8::KeyPress(char key)
{
	auto it = m_validKeys.find(key);
	if (it != m_validKeys.end())
	{
		m_keyPressed = it->second;
	}
}