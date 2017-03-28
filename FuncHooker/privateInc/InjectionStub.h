/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		InjectionStub.h
 *  \author		Andrew Shurney
 *  \brief		A little bit of code for the trampoline storage
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef INJECTION_STUB_H
#define INJECTION_STUB_H

#include "ASMStubs.h"

#ifdef _MSC_VER
# define PACK_ATTR
#else
# define PACK_ATTR  __attribute__((__packed__))
#endif

#ifdef _MSC_VER
# pragma pack(push, 1)
#endif


// On call, first go to my function which takes same arguments.
// then pass them in the second time. 

struct InjectionStub
{
#if defined(X64) || defined(WIN64)
	byte funcHeader[128]; // All code rewrites considered, the absolute worst case scenario (a 14 byte long jump overwriting a table of 7 conditional short jumps each of which then need to be rewritten into long jumps) yields a maximum header size of 126 bytes. 2 bytes for padding because I like round numbers.
	ASM::LJmp executeInjectee;
	ASM::LJmp executeInjector;
#else
	uint8_t funcHeader[32];  // All code rewrites considered, the absolute worst case scenario (a 5 byte jump overwriting a table of 2 conditional short jumps each of which then need to be rewritten into regular conditional jumps) then a 13 byte instruction yields a maximum header size of 23 bytes. A few bytes for padding because I like round numbers.
	ASM::Jmp executeInjectee;
#endif

	InjectionStub(void *FunctionPtr, void *InjectionPtr, unsigned overwriteSize);
} PACK_ATTR;

#ifdef _MSC_VER
# pragma pack(pop)
#endif

#undef PACK_ATTR

#endif
