/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		Operand.h
 *  \author		Andrew Shurney
 *  \brief		Manages a single instruction operand
 */

#ifndef OPERAND_H
#define OPERAND_H

#include <cstdint>
#include "udis86.h"

class Operand
{
	public:
		typedef enum ud_type Type;

		struct Ptr
		{
			unsigned segment;
			unsigned offset;
		};

	private:
		friend class Operation;
		typedef struct ud_operand ud_op;

		Type type;
		unsigned size;
		uint64_t value;
		Type base;
		Type index;
		unsigned offset;
		unsigned scale;

		Operand(const ud_op& operand);

	public:
		const Type& GetType() const;
		const Type& GetBaseReg() const;
		const Type& GetIndexReg() const;

		unsigned GetSize() const;
		unsigned GetIndexScale() const;
		unsigned GetIndexOffset() const;

		template<typename T>
		T GetValue() const
		{
			return *reinterpret_cast<const T*>(&value);
		}

		Ptr GetValue() const
		{
			Ptr ptr;
			ptr.segment = static_cast<unsigned>(value >> 32);
			ptr.offset = static_cast<unsigned>(value & 0xFFFFFFFF);

			return ptr;
		}
};

#endif
