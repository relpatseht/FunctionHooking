/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Operation.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a single machine code instruction
 */

#include <new>
#include <cassert>
#include "privateInc/Operand.h"
#include "privateInc/Operation.h"

Operation::Operation(ud_t *disasm) : size(ud_disassemble(disasm)), numOperands(3), operands(NULL), prefixBytes(), mnemonic(disasm->mnemonic)
{
	if(disasm->operand[2].type == UD_NONE)
		--numOperands;
	if(disasm->operand[1].type == UD_NONE)
		--numOperands;
	if(disasm->operand[0].type == UD_NONE)
		--numOperands;

	if(numOperands)
	{
		uint8_t *operandMem = new uint8_t[sizeof(Operand)*numOperands];

		for(unsigned o=0; o<numOperands; ++o)
			new (operandMem + o*sizeof(Operand)) Operand(disasm->operand[o]);

		operands = reinterpret_cast<Operand*>(operandMem);
	}

	prefixBytes.rex   = disasm->pfx_rex;
	prefixBytes.seg   = disasm->pfx_seg;
	prefixBytes.opr   = disasm->pfx_opr;
	prefixBytes.adr   = disasm->pfx_adr;
	prefixBytes.lock  = disasm->pfx_lock;
	prefixBytes.rep   = disasm->pfx_rep;
	prefixBytes.repe  = disasm->pfx_repe;
	prefixBytes.repne = disasm->pfx_repne;

	prefixBytes.modRM = disasm->modrm;
	prefixBytes.hasModRM = disasm->have_modrm != 0;
}

Operation::Operation(const Operation &rhs) : size(rhs.size), numOperands(rhs.numOperands), operands(NULL), prefixBytes(rhs.prefixBytes), mnemonic(rhs.mnemonic)
{
	if(numOperands)
	{
		uint8_t *operandMem = new uint8_t[sizeof(Operand)*numOperands];

		for(unsigned o=0; o<numOperands; ++o)
			new (operandMem + o*sizeof(Operand)) Operand(rhs.operands[o]);

		operands = reinterpret_cast<Operand*>(operandMem);
	}
}

Operation::Operation(Operation&& rhs) : size(rhs.size), numOperands(rhs.numOperands), operands(rhs.operands),
	                                          prefixBytes(rhs.prefixBytes), mnemonic(rhs.mnemonic)
{
	rhs.numOperands = 0;
	rhs.operands = NULL;
}

Operation::~Operation()
{
	Destroy();
}

void Operation::Destroy()
{
	for(unsigned o=0; o<numOperands; ++o)
		operands[o].~Operand();

	uint8_t *operandMem = reinterpret_cast<uint8_t*>(operands);
	delete [] operandMem;

	numOperands = 0;
	operands = NULL;
}


Operation& Operation::operator=(const Operation& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();

	size = rhs.size;
	numOperands = rhs.numOperands;

	if(numOperands)
	{
		uint8_t *operandMem = new uint8_t[sizeof(Operand)*numOperands];

		for(unsigned o=0; o<numOperands; ++o)
			new (operandMem + o*sizeof(Operand)) Operand(rhs.operands[o]);

		operands = reinterpret_cast<Operand*>(operandMem);
	}

	prefixBytes = rhs.prefixBytes;
	mnemonic = rhs.mnemonic;

	return *this;
}

const Operation::PrefixBytes& Operation::GetPrefixBytes() const
{
	return prefixBytes;
}

unsigned Operation::GetNumOperands() const
{
	return numOperands;
}

const Operand& Operation::GetOperand(unsigned op) const
{
	assert(op < numOperands && "Operand out of range.");
	return operands[op];
}

unsigned Operation::GetSize() const
{
	return size;
}

const Operation::Mnemonic& Operation::GetMnemonic() const
{
	return mnemonic;
}
