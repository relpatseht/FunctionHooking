/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Disassembler.h
 *  \author		Andrew Shurney
 *  \brief		Dissassembles x86 and x64 machine code
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <cstdint>
#include "Operation.h"

class Disassembler
{
	private:
		typedef struct ud ud_t;

		ud_t *disasm;
		uint8_t *curPos;

		static int NextFuncByte(ud_t* disasm);

	public:
		Disassembler(void *addr = NULL);
		Disassembler(const Disassembler& rhs);
		Disassembler(Disassembler&& rhs);
		~Disassembler();

		Disassembler& operator=(const Disassembler& rhs);

		template<typename T>
		T GetIP() const
		{
			return reinterpret_cast<T>(curPos);
		}

		void *GetIP() const;
		void SetIP(void *addr);

		Operation ReadNextOperation();
};

#endif
