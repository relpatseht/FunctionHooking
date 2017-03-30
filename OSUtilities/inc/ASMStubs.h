/************************************************************************************\
* OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ASMStubsX64.h
*  \author		Andrew Shurney
*  \brief		X86 and X64 machine code instructions as structures.
*/

#ifndef ASM_STUBS_X64_H
#define ASM_STUBS_X64_H

#include <cstdint>

#ifdef _MSC_VER
# define PACK_ATTR
#else
# define PACK_ATTR  __attribute__((__packed__))
#endif

#ifdef _MSC_VER
# pragma pack(push, 1)
#endif //#ifdef _MSC_VER

namespace ASM
{
	namespace REG
	{
		enum SIMD : uint8_t
		{
			XMM0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15
		};

		enum CPuint64_t : uint8_t
		{
			RAX,
			RCX,
			RDX,
			RBX,
			RSP,
			RBP,
			RSI,
			RDI,
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15
		};

		enum CPU86 : uint8_t
		{
			EAX,
			ECX,
			EDX,
			EBX,
			ESP,
			EBP,
			ESI,
			EDI,
		};
	}

	namespace MOD
	{
		enum
		{
			PTR,
			PTR_DISP8,
			PTR_DISP32,
			VAL
		};
	}

	/* A ModR/M uint8_t is decoded as follows:

	MOD<7-6>, REG<5-3>, R/M<2-0>

	MOD:
	- 00 (0 - PTR):
	- R/M = 100: Register indicrect addresing mode / SIB with no displacement
	- R/M = 101: Displacement only addressing mode
	- 01 (1 - PTR_DISP8): int8_t displacement follows addressing uint8_t(s) (SIB)
	- 10 (2 - PTR_DISP32): int32_t displacement follows addressing uint8_t(s) (SIB)
	- 11 (3 - VAL): Register addressing mode

	REG:
	- Source or destination register, depending on opcode

	R/M:
	- The operand. Often a register
	*/
	struct ModRM
	{
		uint8_t modRM;

		ModRM(uint8_t mod, uint8_t reg, uint8_t rm);
		ModRM(uint8_t value);
	} PACK_ATTR;

	/* Paired with ModR/M as addressing mode bytes (Scaled Indexed addressing mode Byte)

	SCALE<7-6>, INDEX<5-3>, BASE<2-0>

	Address = BASE + INDEX*(2^SCALE);
	BASE:
	- Register
	- 101: Displacement only if MOD of ModR/M = 00, EBP otherwise

	INDEX:
	- Register
	- 100 illegal
	*/
	struct SIB
	{
		uint8_t sib;

		SIB(uint8_t scale, uint8_t index, uint8_t base);
		SIB(uint8_t value);
	} PACK_ATTR;

	struct REX
	{
		uint8_t rex;

		REX(bool _64bit, bool regPrefix, bool indexPrefix, bool rmBasePrefix);
		REX(uint8_t value);
	} PACK_ATTR;

	struct MovRegToStackWithS8Offset_X64
	{
		REX rex;
		uint8_t movOpcode;
		ModRM modRM;
		SIB sib;
		int8_t offset;

		MovRegToStackWithS8Offset_X64(int8_t offset, uint8_t srcReg = REG::RAX);
	} PACK_ATTR;

	struct MovU32ToStackWithS8Offset
	{
		uint8_t movOpcode;
		ModRM modRM;
		SIB sib;
		int8_t offset;
		uint32_t value;

		MovU32ToStackWithS8Offset(uint32_t value, int8_t offset);
	} PACK_ATTR;

	struct AndRegU32
	{
		uint8_t andOpcode;
		uint8_t ext;
		uint32_t operand;

		AndRegU32(uint32_t operand, uint8_t reg);
	} PACK_ATTR;

	struct PushU32
	{
		uint8_t opcode;
		uint32_t value;

		PushU32();
		PushU32(uint32_t value);
		PushU32(float value);
	} PACK_ATTR;

	struct PushReg
	{
		uint8_t pushOpcode;

		PushReg(uint8_t reg);
	} PACK_ATTR;

	struct PopReg
	{
		uint8_t popOpcode;

		PopReg(uint8_t reg);
	} PACK_ATTR;

	struct Pushuint64_t_X64
	{
		PushU32 lowVal;
		uint8_t movOpcode; // The push operation only takes a 32 bit value, but actually
		ModRM modRM;  // pushes 8 bytes on the stack. So after the push, we need to
		SIB sib;      // mov the upper 4 bytes into the correct position
		uint8_t rspOffset;
		uint32_t highVal;

		Pushuint64_t_X64(uint64_t value);
		Pushuint64_t_X64(double value);
	} PACK_ATTR;

	struct Pushuint64_t_X86
	{
		PushU32 highVal;
		PushU32 lowVal;

		Pushuint64_t_X86(uint64_t value);
		Pushuint64_t_X86(double value);
	} PACK_ATTR;

	struct Return
	{
		uint8_t retOpcode;

		Return();
	} PACK_ATTR;

	struct NOP
	{
		uint8_t nopOpcode;

		NOP();
	};

	struct Jmp
	{
		uint8_t opcode;
		int32_t relativeOffset;

		Jmp(const void *from, const void *to);
		Jmp(int32_t offset);
	} PACK_ATTR;

	struct SJmp
	{
		uint8_t opcode;
		int8_t offset;

		SJmp(const void *from, const void *to);
		SJmp(int8_t offset);
	} PACK_ATTR;

	/* A jump to an absolute address cannot be done with an immediate value
	and we don't want to touch a register. The solution is to push the
	lower half of the value, then move the upper half on top of, then return

	This is only for use in 64 bit code.
	*/
	struct LJmp
	{
		Pushuint64_t_X64 addr;
		Return ret;

		LJmp(const void *addr);
	} PACK_ATTR;

	struct MovToReg_X64
	{
		REX rex;
		uint8_t movOpcode;
		uint64_t value;

		MovToReg_X64(uint64_t value, uint8_t reg = REG::RAX);
	} PACK_ATTR;

	struct MovToReg_X86
	{
		uint8_t movOpcode;
		uint32_t value;

		MovToReg_X86(uint32_t value, uint8_t reg = REG::RAX);
	} PACK_ATTR;

	struct MovEAXToAddr
	{
		uint8_t movOpcode;
		uint32_t addr;

		MovEAXToAddr(const void *addr);
	} PACK_ATTR;

	struct MovRAXToAddr
	{
		REX rex;
		uint8_t movOpcode;
		uint64_t addr;

		MovRAXToAddr(const void *addr);
	} PACK_ATTR;

	struct MovPtrSSEReg
	{
		uint8_t sseOpcode;
		uint8_t sseOpcodePrefix;
		uint8_t movOpcode;
		ModRM modRM;
		int32_t valOffset;

		MovPtrSSEReg(int32_t valOffset, uint8_t reg, bool regToMem, bool dbl);
	} PACK_ATTR;

	struct MovF32PtrToSSEReg
	{
		MovPtrSSEReg mov;

		MovF32PtrToSSEReg(int32_t floatOffset, uint8_t reg = REG::XMM0);
		MovF32PtrToSSEReg(const void *valAddr, const void *instrAddr, uint8_t reg = REG::XMM0);
	} PACK_ATTR;

	struct MovF64PtrToSSEReg
	{
		MovPtrSSEReg mov;

		MovF64PtrToSSEReg(int32_t dblOffset, uint8_t reg = REG::XMM0);
		MovF64PtrToSSEReg(const void *valAddr, const void *instrAddr, uint8_t reg = REG::XMM0);
	} PACK_ATTR;

	struct MovSSERegToF32Ptr
	{
		MovPtrSSEReg mov;

		MovSSERegToF32Ptr(int32_t floatOffset, uint8_t reg = REG::XMM0);
		MovSSERegToF32Ptr(const void *valAddr, const void *instrAddr, uint8_t reg = REG::XMM0);
	} PACK_ATTR;

	struct MovSSERegToF64Ptr
	{
		MovPtrSSEReg mov;

		MovSSERegToF64Ptr(int32_t dblOffset, uint8_t reg = REG::XMM0);
		MovSSERegToF64Ptr(const void *valAddr, const void *instrAddr, uint8_t reg = REG::XMM0);
	} PACK_ATTR;

	struct FldF32Ptr_X86
	{
		uint8_t fldOpcode;
		ModRM modRM;
		uint32_t addr;

		FldF32Ptr_X86(const void *valAddr);
	} PACK_ATTR;

	struct FldF64Ptr_X86
	{
		uint8_t fldOpcode;
		ModRM modRM;
		uint32_t addr;

		FldF64Ptr_X86(const void *valAddr);
	} PACK_ATTR;

	struct FstpF32Ptr_X86
	{
		uint8_t fstpOpcode;
		ModRM modRM;
		uint32_t addr;

		FstpF32Ptr_X86(const void *valAddr);
	} PACK_ATTR;

	struct FstpF64Ptr_X86
	{
		uint8_t fstpOpcode;
		ModRM modRM;
		uint32_t addr;

		FstpF64Ptr_X86(const void *valAddr);
	} PACK_ATTR;

	struct CallReg_X64
	{
		REX rex;
		uint8_t callOpcode;
		ModRM modRM;

		CallReg_X64(uint8_t reg = REG::RAX);
	} PACK_ATTR;

	struct CallReg_X86
	{
		uint8_t callOpcode;
		ModRM modRM;

		CallReg_X86(uint8_t reg = REG::EAX);
	} PACK_ATTR;

#if defined(X64) || defined(WIN64)
	typedef Pushuint64_t_X64 Pushuint64_t;
	typedef MovToReg_X64 MovToReg;
	typedef CallReg_X64 CallReg;
#else
	typedef Pushuint64_t_X86 Pushuint64_t;
	typedef MovToReg_X86 MovToReg;
	typedef CallReg_X86 CallReg;
#endif

	struct CallAddr
	{
		MovToReg addrEax;
		CallReg callEax;

		CallAddr(const void *addr);
	} PACK_ATTR;

	struct AddS8_X64
	{
		REX rex;
		uint8_t addOpcode;
		ModRM modRM;
		int8_t value;

		AddS8_X64(int8_t value, uint8_t reg = REG::RAX);
	} PACK_ATTR;

	struct AddS8_X86
	{
		uint8_t addOpcode;
		ModRM modRM;
		int8_t value;

		AddS8_X86(int8_t value, uint8_t reg = REG::EAX);
	} PACK_ATTR;

	/*! In x86, 0x40 increments eax by 1. In x64, it is a rex prefix.
	So, on x86, this will result in eax being 1, while on x64, it will
	be 0. (Rex prefix to a nop still does nothing.) */
	struct IsX86
	{
		MovToReg clearEax;
		uint8_t incOrRexOpcode;
		NOP nop;

		IsX86();
	} PACK_ATTR;

#if defined(X64) || defined(WIN64)
	typedef AddS8_X64 AddS8;
#else
	typedef AddS8_X86 AddS8;
#endif
}

#ifdef _MSC_VER
# pragma pack(pop)
#endif //#ifdef _MSC_VER

#undef PACK_ATTR

#endif //#ifndef ASM_STUBS_X64_H
