/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		FuncHookerCPP.h
 *  \author		Andrew Shurney
 *  \brief		Internal C++ func hooker object
 */

#ifndef FUNC_HOOKER_CPP_H
#define FUNC_HOOKER_CPP_H

#include <cstdint>

struct InjectionStub;
class  Disassembler;
class DynamicCodeAllocator;

struct FuncHooker
{
	private:
		typedef unsigned char uint8_t;

		struct DeadZone
		{
			uint8_t *addr;
			unsigned len;

			DeadZone(uint8_t *addr=NULL, unsigned len=0);
		};

		static DynamicCodeAllocator *stubArea;
		static unsigned instances;

		bool installed;
		InjectionStub *stubCode;
		void* InjectionFunc;

		uint8_t* injectionJumpTarget;
		unsigned overwriteSize;

		uint8_t* backupCode;
		uint8_t* proxyBackupCode;
		unsigned proxyBackupCodeSize;
		unsigned backupCodeSize;

		bool hotpatchable;

		Disassembler *disasm;
		uint8_t *funcPtr;

		void FindFunctionBody();
		DeadZone FindNearestDeadZone(uint8_t *start, unsigned delta, unsigned minSize = 0);
		bool PrepareFunctionForHook();
		void InstallProxy(const DeadZone& zone, void *stubDist, void *injectDist, uint8_t *stubMem, unsigned deadZoneMinSize);
		bool RelocateFunctionHeader(unsigned headerSize);

		FuncHooker(const FuncHooker&);            // Do not implement
		FuncHooker& operator=(const FuncHooker&); // Do not implement

	public:
		FuncHooker(void *FunctionPtr, void *InjectionPtr);
		virtual ~FuncHooker();

		const void *GetTrampoline() const;

		bool InstallHook();
		void RemoveHook();

};

#endif