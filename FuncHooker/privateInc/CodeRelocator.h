/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		CodeRelocator.h
 *  \author		Andrew Shurney
 *  \brief		Moves instructions from one place to another keeping relative
 *              offsets intact
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef CODE_RELOCATOR_H
#define CODE_RELOCATOR_H

#include <cstdint>

class Operation;
class Operand;

class CodeRelocator
{
	private:
		uint8_t *from;
		uint8_t *to;
		unsigned codeSize;
		uint8_t *curFrom;
		uint8_t *curTo;

		intptr_t RelocateOffset(intptr_t offset) const;
		unsigned GetOffsetSize(intptr_t offset) const;
		intptr_t GetOperandRelOffset(const Operand& oper) const;
		intptr_t GetOperandOffset(const Operand& oper) const;

		void CopyOperation(unsigned operSize);

		/* This function is called in the case we are moving an operation
		   with a constant relative offset, but the resulting position is
		   still within the section of code we are moving.
		*/
		void MoveRelInstrWithTarget(const Operation& operation, const Operand &operand);

		/* This function is called in the case we are moving an opertion
		   with a constant relative offset and the resulting offset will
		   be larger than +/-2gb, meaning we'll need to use a 64 bit
		   offset rather than a 32 bit one.
		*/
		void MoveRelInstrFar(const Operation& operation, const Operand &operand);

		/* This function handles moving an operation with an 8bit constant
		   relative offset. We'll need to translate this to the appropriate
		   32 bit operation.
		*/
		void MoveShortRelInstr(const Operation& operation, const Operand &operand);

		/* This function handles the case where we just need to relocate a
		   32 bit offset. How easy!
		*/
		void MoveRelInstr(const Operand &operand, unsigned operSize);

	public:
		CodeRelocator(void *fromBaseAddr, void *toBaseAddr, unsigned codeSize = 0);

		bool Relocate(const Operation& operation);
};

#endif
