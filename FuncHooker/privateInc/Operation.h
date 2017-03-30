/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Operation.h
 *  \author		Andrew Shurney
 *  \brief		Manages a single machine code instruction
 */

#ifndef OPERATION_H
#define OPERATION_H

#include <cstdint>
#include "udis86.h"
#include "Operand.h"

class Operation
{
	public:
		typedef enum ud_mnemonic_code Mnemonic;

		struct PrefixBytes
		{
			uint8_t rex;
			uint8_t seg;
			uint8_t opr;
			uint8_t adr;
			uint8_t lock;
			uint8_t rep;
			uint8_t repe;
			uint8_t repne;
			uint8_t modRM;
			bool hasModRM;
		};

	private:
		friend class Disassembler;
		typedef struct ud ud_t;

		unsigned size;
		unsigned numOperands;
		Operand *operands;

		PrefixBytes prefixBytes;
		Mnemonic mnemonic;

		Operation(ud_t *disasm);

		void Destroy();

	public:
		Operation(const Operation &rhs);
		Operation(Operation&& rhs);
		~Operation();

		Operation& operator=(const Operation& rhs);

		const PrefixBytes& GetPrefixBytes() const;

		unsigned GetNumOperands() const;
		const Operand& GetOperand(unsigned op = 0) const;

		unsigned GetSize() const;

		const Mnemonic& GetMnemonic() const;
};

#endif
