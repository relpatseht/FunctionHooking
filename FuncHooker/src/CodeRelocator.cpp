/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		CodeRelocator.cpp
 *  \author		Andrew Shurney
 *  \brief		Moves instructions from one place to another keeping relative
 *              offsets intact
 */

#include <exception>
#include <cstring>
#include <new>
#include "ASMStubs.h"
#include "privateInc/CodeRelocator.h"
#include "privateInc/Operation.h"

CodeRelocator::CodeRelocator(void *fromBaseAddr, void *toBaseAddr, unsigned codeSize) :
                             from(reinterpret_cast<uint8_t*>(fromBaseAddr)),
							 to(reinterpret_cast<uint8_t*>(toBaseAddr)),
							 codeSize(codeSize), curFrom(from), curTo(to)
{
}

bool CodeRelocator::Relocate(const Operation& operation)
{
	unsigned operSize = operation.GetSize();

	for(unsigned op=0; op < operation.GetNumOperands(); ++op)
	{
		const Operand& operand = operation.GetOperand(op);

		/* We know an operation can only have 1 operand we care about.
		   Thanks to that, we don't need to store modifications from previous
		   operands and can return as soon as 1 modification takes place.
		   This really simplifies the code, as we can assume no modifications
		   have taken place yet.
		*/
		switch(operand.GetType())
		{
			case UD_OP_JIMM:{ // Relative offset (loops, jumps, calls)
				intptr_t offset = GetOperandRelOffset(operand);
				intptr_t movedOffset = RelocateOffset(offset);
        int64_t farOffset64 = 1;
        farOffset64 <<= 31;
        farOffset64 -= 1;
        intptr_t farOffset  = static_cast<intptr_t>(farOffset64);

				uint8_t *offsetTarget = curFrom + operSize + offset;
				if(offsetTarget >= from && offsetTarget < from + codeSize)
					MoveRelInstrWithTarget(operation, operand);
				else if(std::abs(movedOffset) > farOffset)
					MoveRelInstrFar(operation, operand);
				else if(operand.GetSize() == 8)
					MoveShortRelInstr(operation, operand);
				else
					MoveRelInstr(operand, operSize);

				curFrom += operSize;
				return true;
			}break;
			case UD_OP_MEM: // Register + offset ptr (eg: [RIP+0x2])
				if(operand.GetBaseReg() == UD_R_RIP || operand.GetIndexReg() == UD_R_RIP) // Handle RIP relative addressing.
				{
					int32_t offset = static_cast<int32_t>(GetOperandOffset(operand));
					int32_t scale = (operand.GetIndexReg() == UD_R_RIP) ? (1 << operand.GetIndexScale()) : 1;
					intptr_t target = reinterpret_cast<intptr_t>(curFrom)*scale + offset;
					intptr_t newOffset = target - reinterpret_cast<intptr_t>(curTo);

					// Sadly, our new offset is just to big. We cannot relate to this new address.
					if(std::abs(newOffset) > (1u<<31) - 1)
						return false;

					// I really should recompile the instruction, but for that I would
					// basically have to build and integrate an assembler, which I believe
					// wouldn't be worth the effort.
					// Instead, I'll just copy over the instruction and look for the old
					// offset starting from the back. When (if) I find it, I'll overwrite
					// it with the new one.

					uint8_t *operStart = curTo;
					CopyOperation(operSize);
					curFrom += operSize;

					// We can add only go to 2 bytes after the start of the operation,
					// as because of the REX prefix and the opcode we're guaranteed
					// those CANNOT be our offset.
					for(uint8_t *curPos = curTo - sizeof(int32_t); curPos > operStart+1; --curPos)
					{
						if(*((int32_t*)curPos) == offset)
						{
							*((int32_t*)curPos) = static_cast<int32_t>(newOffset);
							return true;
						}
					}

					return false; // If we got here, we didn't find our offset.
				}
			break;
			default:
                continue;
		}
	}

	// We've finished this operation. Copy it over and return
	CopyOperation(operSize);
	curFrom += operSize;
	return true;
}


intptr_t CodeRelocator::RelocateOffset(intptr_t offset) const
{
	intptr_t diff = curTo - curFrom;
	return offset + diff;
}

unsigned CodeRelocator::GetOffsetSize(intptr_t offset) const
{
	if(!(offset & 0xFF))
		return 1;

	if(!(offset & 0xFFFF))
		return 2;

	if(!(offset & 0xFFFFFFFF))
		return 4;

	return 8;
}

intptr_t CodeRelocator::GetOperandRelOffset(const Operand& oper) const
{
	return oper.GetValue<intptr_t>();
}

intptr_t CodeRelocator::GetOperandOffset(const Operand& oper) const
{
	return oper.GetValue<intptr_t>();
}

void CodeRelocator::CopyOperation(unsigned operSize)
{
	std::memcpy(curTo, curFrom, operSize);

	curTo += operSize;
}

void CodeRelocator::MoveRelInstrWithTarget(const Operation& operation, const Operand &operand)
{
	unsigned operSize = operation.GetSize();

	// We only care about relative calls here, which mean we need they
	// are storing an instruction pointer which we will need to alter.
	switch(operation.GetMnemonic())
	{
		case UD_Icall:{
			intptr_t offset = GetOperandRelOffset(operand);

			if(offset < 0)
				throw std::exception(/*"Negative relative call pointing within the target area unsupported."*/);

			// The call to 0 or just jumping a nop. Change it to a push and be done with it.
			new (curTo) ASM::PushU32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(curFrom)) + 5); // 5 is the size of the relative call
			curTo += sizeof(ASM::PushU32);

			if(offset >= 2) // we also need to jump to the offset
			{
				if(offset > 127) // we need to use a regular jump
				{
					new (curTo) ASM::Jmp(curTo, curTo + offset);
					curTo += sizeof(ASM::Jmp);
				}
				else
				{
					new (curTo) ASM::SJmp(static_cast<int8_t>(offset));
					curTo += sizeof(ASM::SJmp);
				}
			}
		}break;
		default:
			CopyOperation(operSize);
			curTo += operSize;
	}
}

void CodeRelocator::MoveRelInstrFar(const Operation& operation, const Operand &operand)
{
#if defined(X64) || defined(WIN64)
	unsigned operSize = operation.GetSize();

	switch(operation.GetMnemonic())
	{
		case UD_Icall:{
			// A call is just a push and a jump. So push our new return address (just past the long jump)
			uint64_t returnAddr = reinterpret_cast<uint64_t>(curTo) + sizeof(ASM::Pushuint64_t) + sizeof(ASM::LJmp);
			new (curTo) ASM::Pushuint64_t(returnAddr);
			curTo += sizeof(ASM::Pushuint64_t);
		}break;
		case UD_Ijmp:
			// Unconditional jump. This trivial case only requires a long jump, so do nothing here
		break;
		default:
			// All other jumps are on some condition. Lets just be a little tricky.
			// We'll change these jumps offsets to be jumping to our long jump over
			// an unconditional short jump which jumps over the long jump. That way,
			// if the condition is satisfied, we jump, otherwise, we skip it.

			CopyOperation(operSize);                  // Start by just copying the operaion over

			// All 32 bit conditional jumps follow a pattern. Their only difference
			// from their 8 bit sisters are they are preceeded by a 0x0F byte and
			// their opcode is 0x10 larger. Lets convert 32 bit operations to 8 bit
			// if we can for efficiencies sake.
			if(operand.GetSize() == 32)
			{
				curTo -= sizeof(uint32_t) + 1; // Back up to the operand
				uint8_t operand = *curTo--;    // Grab the operand and shift back to the 0xF byte.
				*curTo = operand - 0x10;  // Write in the 8bit operand
				curTo += 2;               // Shift curTo to where it would have been had we read an 8bit jmp from the start
			}

			*(curTo - 1) = sizeof(ASM::SJmp);         // And make it a jump over a single short jump

			new (curTo) ASM::SJmp(sizeof(ASM::LJmp)); // Add in our unconditional jump over the long jump
			curTo += sizeof(ASM::SJmp);
	}

	// Now that we've gotten the clever hacks out of the way, we can preform our long jump.

	intptr_t offset = GetOperandRelOffset(operand); // Get the offset we were supposed to go.
	new (curTo) ASM::LJmp(curFrom + offset); // And make an absolute address out of it.
	curTo += sizeof(ASM::LJmp);
#else
	bool clearWarning = operation.GetSize() == operation.GetSize() && operand.GetSize() == operand.GetSize();
	if(clearWarning)
		throw std::exception("Making a Far move on 32 bit. This shouldn't be possible.");
#endif
}

void CodeRelocator::MoveShortRelInstr(const Operation& operation, const Operand &operand)
{
	unsigned operSize = operation.GetSize();
	intptr_t newOffset = RelocateOffset(GetOperandRelOffset(operand));

	switch(operation.GetMnemonic())
	{
		case UD_Ijmp:
			new (curTo) ASM::Jmp(static_cast<int32_t>(newOffset)); // Converting a short jump to a regular jump is trivial.
			curTo += sizeof(ASM::Jmp);
		break;
		case UD_Ijcxz: case UD_Ijrcxz: case UD_Ijecxz:
		case UD_Iloop: case UD_Iloope: case UD_Iloopne:
			// All these instructions are conditional jumps with no 32 bit counterpart.
			// Let's instead made them jump to a jump to our new offset right over a
			// jump past our jump to the new offset, thus keeping the condititon and
			// getting a jump with larger range.

			CopyOperation(operSize);          // Start by copying over the operation
			*(curTo - 1) = sizeof(ASM::SJmp); // Then change the offset to be over a short jump and to our regular jump

			new (curTo) ASM::SJmp(sizeof(ASM::Jmp)); // If our condition failed, jump over our regular jump
			curTo += sizeof(ASM::SJmp);

			new (curTo) ASM::Jmp(static_cast<int32_t>(newOffset)); // Now, we can safely jump to our new offset only if the condition succeeded.
			curTo += sizeof(ASM::Jmp);
		break;
		default:
			// All other 8 bit conditional jumps share a pattern. To get to their 32
			// bit counterpart, all you need to do is preceed the opcode with a 0x0F
			// byte and add 0x10 to the opcode itself.

			CopyOperation(operSize);      // Copy over the operation (to save any potential prefix bytes)

			curTo -= 2;                   // Backup our write pointer to the opcode.
			uint8_t opcode = *curTo + 0x10;    // Make the new 32 bit opcode

			*curTo++ = 0x0F;              // Write in the 0xF byte.
			*curTo++ = opcode;            // Then the 32 bit version of the opcode.

			*((int32_t*)curTo++) = static_cast<int32_t>(newOffset); // Now we can safely write in our offset
	}
}

void CodeRelocator::MoveRelInstr(const Operand &operand, unsigned operSize)
{
	intptr_t newOffset = RelocateOffset(GetOperandRelOffset(operand));

	CopyOperation(operSize);      // Copy over the operation
	curTo -= sizeof(int32_t);         // Backup curTo to where we write the address.

	*((int32_t*)curTo++) = static_cast<int32_t>(newOffset); // And write in the new address
}
