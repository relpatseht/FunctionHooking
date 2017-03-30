/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Disassembler.cpp
 *  \author		Andrew Shurney
 *  \brief		Dissassembles x86 and x64 machine code
 */

#include "privateInc/Disassembler.h"
#include "udis86.h"

Disassembler::Disassembler(void *addr) : disasm(new ud_t), curPos(reinterpret_cast<uint8_t*>(addr))
{
	ud_init(disasm);
	ud_set_mode(disasm, sizeof(void*)*8);
	ud_set_user_opaque_data(disasm, this);
	ud_set_input_hook(disasm, &Disassembler::NextFuncByte);
}

Disassembler::Disassembler(const Disassembler& rhs) : disasm(new ud_t), curPos(rhs.curPos)
{
	ud_init(disasm);
	ud_set_mode(disasm, sizeof(void*)*8);
	ud_set_user_opaque_data(disasm, this);
	ud_set_input_hook(disasm, &Disassembler::NextFuncByte);

	*this = rhs;
}

Disassembler::Disassembler(Disassembler&& rhs) : disasm(rhs.disasm), curPos(rhs.curPos)
{
	rhs.disasm = NULL;
}

Disassembler::~Disassembler()
{
	delete disasm;
}

Disassembler& Disassembler::operator=(const Disassembler& rhs)
{
	if(this == &rhs)
		return *this;

	curPos = rhs.curPos;

	return *this;
}

void *Disassembler::GetIP() const
{
	return curPos;
}

void Disassembler::SetIP(void *addr)
{
	curPos = reinterpret_cast<uint8_t*>(addr);
}

Operation Disassembler::ReadNextOperation()
{
	return Operation(disasm);
}

int Disassembler::NextFuncByte(ud_t* disasm)
{
	Disassembler *thisPtr = (Disassembler*)ud_get_user_opaque_data(disasm);
	return *thisPtr->curPos++;
}
