/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		InjectionStub.cpp
 *  \author		Andrew Shurney
 *  \brief		A little bit of code for the trampoline storage
 */

#include <cstring>
#include "privateInc/InjectionStub.h"

#ifdef _MSC_VER
# pragma warning(disable : 4355)
#endif

template<typename C, typename T>
static uintptr_t GetOffset(T C::*member)
{
	T* ptr = &(reinterpret_cast<C*>(NULL)->*member);
	return reinterpret_cast<uintptr_t>(ptr);
}

InjectionStub::InjectionStub(void *FunctionPtr, void *InjectionPtr, unsigned overwriteSize) :
#if defined(X64) || defined(WIN64)
                             executeInjectee(reinterpret_cast<uint8_t*>(FunctionPtr) + overwriteSize), // Absolute address :) 
							 executeInjector(reinterpret_cast<uint8_t*>(InjectionPtr))                 // This is only ever used if we needed a long proxy
#else
							 executeInjectee(reinterpret_cast<uint8_t*>(this) + GetOffset(&InjectionStub::executeInjectee),
								             reinterpret_cast<uint8_t*>(FunctionPtr) + overwriteSize)  // Offset FuntionPtr by overwriteSize so we don't infinite loop our header function
#endif
{
	// Fill the header with nops so it's safe to execute
	const uint8_t nop = 0x90;
	std::memset(funcHeader, nop, sizeof(funcHeader));

	InjectionPtr = InjectionPtr;
}
