/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ASMStubs.cpp
 *  \author		Andrew Shurney
 *  \brief		X86 and X64 machine code instructions as structures.
 */

#include "ASMStubs.h"
#include <cassert>

#ifdef _MSC_VER
# pragma warning(disable : 4355)
#endif

using namespace ASM;

template<typename T>
static T GetOffset(const void *from, const void *to, uint32_t instrSize)
{
	intptr_t offset = reinterpret_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(from) - instrSize;
	T castedOffset = static_cast<T>(offset);
	assert(offset == castedOffset && "Offset too large for given type");

	return castedOffset;
}

ModRM::ModRM(uint8_t mod, uint8_t reg, uint8_t rm) : modRM(((mod & 0x3) << 6) | ((reg & 0x7) << 3) | (rm & 0x7)) {}
ModRM::ModRM(uint8_t value) : modRM(value) {}

SIB::SIB(uint8_t scale, uint8_t index, uint8_t base) : sib(((scale & 0x3) << 6) | ((index & 0x7) << 3) | (base & 0x7)) {}
SIB::SIB(uint8_t value) : sib(value) {}

REX::REX(bool _64bit, bool regPrefix, bool indexPrefix, bool rmBasePrefix)
{
	rex = 0x40;

	if(_64bit)
		rex |= 0x8;

	if(regPrefix)
		rex |= 0x4;

	if(indexPrefix)
		rex |= 0x2;

	if(rmBasePrefix)
		rex |= 0x1;
}
REX::REX(uint8_t value) : rex(value) {}

MovU32ToStackWithS8Offset::MovU32ToStackWithS8Offset(uint32_t value, int8_t offset) : movOpcode(0xC7), modRM(MOD::PTR_DISP8, 0, REG::ESP), sib(0, 0x4, REG::ESP), offset(offset), value(value) {}

MovRegToStackWithS8Offset_X64::MovRegToStackWithS8Offset_X64( int8_t offset, uint8_t srcReg ) : rex(true, srcReg >= REG::R8, false, false), movOpcode(0x89), modRM(MOD::PTR_DISP8, srcReg & 0x7, REG::RSP), sib(0, 0x4, REG::RSP), offset(offset){}

AndRegU32::AndRegU32( uint32_t operand, uint8_t reg ) : andOpcode( 0x81 ), ext( 0xe0 + ( reg & 0x7 ) ), operand( operand )
{
	assert( reg < REG::R8 );
}

PushReg::PushReg( uint8_t reg ) : pushOpcode( 0x50 + ( reg & 0x7 ) )
{
	assert( reg < REG::R8 );
}

PopReg::PopReg( uint8_t reg ) : popOpcode( 0x58 + ( reg & 0x7 ) )
{
	assert( reg < REG::R8 );
}

PushU32::PushU32() : opcode(0x68){}
PushU32::PushU32(uint32_t value) : opcode(0x68), value(value) {}
PushU32::PushU32(float value) : opcode(0x68), value(*reinterpret_cast<uint32_t*>(&value)) {}

Pushuint64_t_X64::Pushuint64_t_X64(uint64_t value) : lowVal(static_cast<uint32_t>(value)), 
	                                  movOpcode(0xC7), 
									  modRM(MOD::PTR_DISP8, 0, REG::RSP), 
									  sib(0, REG::RSP, REG::RSP), 
									  rspOffset(0x4), 
						              highVal(static_cast<uint32_t>(value >> 32)){}

Pushuint64_t_X64::Pushuint64_t_X64(double value) : lowVal(), 
	                                  movOpcode(0xC7), 
									  modRM(MOD::PTR_DISP8, 0, REG::RSP), 
									  sib(0, REG::RSP, REG::RSP), 
									  rspOffset(0x4), 
						              highVal()
{
	union
	{
		double f;
		uint64_t u;
	} val;

	val.f = value;
	lowVal.value = static_cast<uint32_t>(val.u);
	highVal = static_cast<uint32_t>(val.u >> 32);
}

Pushuint64_t_X86::Pushuint64_t_X86(uint64_t value) : highVal(static_cast<uint32_t>(value >> 32)),
	                                  lowVal(static_cast<uint32_t>(value)){}

Pushuint64_t_X86::Pushuint64_t_X86(double value) : highVal(),lowVal()
{
	union
	{
		double f;
		uint64_t u;
	} val;

	val.f = value;
	lowVal.value = static_cast<uint32_t>(val.u);
	highVal.value = static_cast<uint32_t>(val.u >> 32);
}

Return::Return() : retOpcode(0xC3) {}

NOP::NOP() : nopOpcode(0x90) {}

Jmp::Jmp(const void *from, const void *to) : opcode(0xE9), 
	                                         relativeOffset(GetOffset<int32_t>(from, to, sizeof(Jmp))) {}

Jmp::Jmp(int32_t offset) : opcode(0xE9), relativeOffset(offset) {}

SJmp::SJmp(const void *from, const void *to) : opcode(0xEB), 
	                                           offset(GetOffset<int8_t>(from, to, sizeof(SJmp))){}

SJmp::SJmp(int8_t offset) : opcode(0xEB), offset(offset){}

LJmp::LJmp(const void *addr) : addr(reinterpret_cast<uint64_t>(addr)),
	                           ret() {}

MovToReg_X64::MovToReg_X64(uint64_t value, uint8_t reg) : rex(true, false, false, reg >= REG::R8), 
                                                movOpcode(0xB8 + (reg & 0x7)), 
										        value(value) {}

MovToReg_X86::MovToReg_X86(uint32_t value, uint8_t reg) :  movOpcode(0xB8 + (reg & 0x7)), 
	                                             value(value) {}

MovEAXToAddr::MovEAXToAddr(const void *addr) : movOpcode(0xA3), 
	                                           addr(reinterpret_cast<uint32_t>(addr)) {}

MovRAXToAddr::MovRAXToAddr(const void *addr) : rex(true, false, false, false), 
	                                           movOpcode(0xA3), 
											   addr(reinterpret_cast<uint64_t>(addr)) {}

MovPtrSSEReg::MovPtrSSEReg(int32_t valOffset, uint8_t reg, bool regToMem, bool dbl) : sseOpcode(dbl ? 0xF2 : 0xF3), 
	                                                                         sseOpcodePrefix(0x0F), 
																		     movOpcode(regToMem ? 0x11 : 0x10),
																		     modRM(0, reg & 0x7, 0x5), // 0x5 = 32bit displacement
																		     valOffset(valOffset) {}

MovF32PtrToSSEReg::MovF32PtrToSSEReg(int32_t floatOffset, uint8_t reg) : mov(floatOffset, reg, false, false) {} 

MovF32PtrToSSEReg::MovF32PtrToSSEReg(const void *valAddr, const void *instrAddr, uint8_t reg) : mov(GetOffset<int32_t>(instrAddr, valAddr, sizeof(MovF32PtrToSSEReg)), reg, false, false) {}

MovF64PtrToSSEReg::MovF64PtrToSSEReg(int32_t dblOffset, uint8_t reg) : mov(dblOffset, reg, false, true) {} 

MovF64PtrToSSEReg::MovF64PtrToSSEReg(const void *valAddr, const void *instrAddr, uint8_t reg) : mov(GetOffset<int32_t>(instrAddr, valAddr, sizeof(MovF64PtrToSSEReg)), reg, false, true) {}

MovSSERegToF32Ptr::MovSSERegToF32Ptr(int32_t floatOffset, uint8_t reg) : mov(floatOffset, reg, true, false) {} 

MovSSERegToF32Ptr::MovSSERegToF32Ptr(const void *valAddr, const void *instrAddr, uint8_t reg) : mov(GetOffset<int32_t>(instrAddr, valAddr, sizeof(MovF32PtrToSSEReg)), reg, true, false) {}

MovSSERegToF64Ptr::MovSSERegToF64Ptr(int32_t dblOffset, uint8_t reg) : mov(dblOffset, reg, true, true) {} 

MovSSERegToF64Ptr::MovSSERegToF64Ptr(const void *valAddr, const void *instrAddr, uint8_t reg) : mov(GetOffset<int32_t>(instrAddr, valAddr, sizeof(MovF32PtrToSSEReg)), reg, true, true) {}

FldF32Ptr_X86::FldF32Ptr_X86(const void *valAddr) : fldOpcode(0xD9), modRM(0, 0, 0x5), addr(reinterpret_cast<uint32_t>(valAddr)) {}

FldF64Ptr_X86::FldF64Ptr_X86(const void *valAddr) : fldOpcode(0xDD), modRM(0, 0, 0x5), addr(reinterpret_cast<uint32_t>(valAddr)) {}

FstpF32Ptr_X86::FstpF32Ptr_X86(const void *valAddr) : fstpOpcode(0xD9), modRM(0, 0x3, 0x5), addr(reinterpret_cast<uint32_t>(valAddr)) {}

FstpF64Ptr_X86::FstpF64Ptr_X86(const void *valAddr) : fstpOpcode(0xDD), modRM(0, 0x3, 0x5), addr(reinterpret_cast<uint32_t>(valAddr)) {}

CallReg_X64::CallReg_X64(uint8_t reg) : rex(true, false, false, false), 
	                               callOpcode(0xFF), 
								   modRM(MOD::VAL, 0x2, reg & 0x7) {}

CallReg_X86::CallReg_X86(uint8_t reg) : callOpcode(0xFF), 
	                               modRM(MOD::VAL, 0x2, reg & 0x7) {}

CallAddr::CallAddr(const void *addr) : addrEax(reinterpret_cast<uintptr_t>(addr), REG::EAX), callEax(REG::EAX) {}

AddS8_X64::AddS8_X64(int8_t value, uint8_t reg) : rex(false, false, false, reg >= REG::R8), 
	                                     addOpcode(0x83), 
										 modRM(MOD::VAL, 0, reg & 0x7), 
										 value(value) {}

AddS8_X86::AddS8_X86(int8_t value, uint8_t reg) : addOpcode(0x83), 
	                                     modRM(MOD::VAL, 0, reg & 0x7), 
										 value(value) {}

IsX86::IsX86() : clearEax(0, REG::EAX), incOrRexOpcode(0x40), nop() {}
