/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Operand.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a single instruction operand
 */

#include "privateInc/Operand.h"

Operand::Operand(const ud_op& operand) : type(operand.type), size(operand.size), value(operand.lval.uqword),
	                                     base(operand.base), index(operand.index), offset(operand.offset), 
										 scale(operand.scale)
{
	unsigned sizeVal = (operand.type == UD_OP_MEM) ? operand.offset : operand.size;

	switch(sizeVal)
	{
		case 8:
			value = *reinterpret_cast<const uint64_t*>(&operand.lval.ubyte);
		break;
		case 16:
			value = *reinterpret_cast<const uint64_t*>(&operand.lval.uword);
		break;
		case 32:
			value = *reinterpret_cast<const uint64_t*>(&operand.lval.udword);
		break;
		case 48:
			value = static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&operand.lval.ptr.seg)) << 32;
			value |= *reinterpret_cast<const uint32_t*>(&operand.lval.ptr.off);
		break;
		case 64:
			value = *reinterpret_cast<const uint64_t*>(&operand.lval.uqword);
		break;
	}
}

const Operand::Type& Operand::GetType() const
{
	return type;
}

const Operand::Type& Operand::GetBaseReg() const
{
	return base;
}

const Operand::Type& Operand::GetIndexReg() const
{
	return index;
}

unsigned Operand::GetSize() const
{
	return size;
}

unsigned Operand::GetIndexScale() const
{
	return scale;
}

unsigned Operand::GetIndexOffset() const
{
	return offset;
}
