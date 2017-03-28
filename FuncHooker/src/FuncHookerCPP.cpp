/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		FuncHookerCPP.cpp
 *  \author		Andrew Shurney
 *  \brief		Internal C++ func hooker object
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdint>
#include "PageManager.h"
#include "privateInc/CodeRelocator.h"
#include "privateInc/DynamicCodeAllocator.h"
#include "privateInc/InjectionStub.h"
#include "privateInc/Disassembler.h"
#include "PriorityBlock.h"
#include "SingleThreadBlock.h"
#include "PrivelegeBlock.h"
#include "ASMStubs.h"
#include "privateInc/FuncHookerCPP.h"

template<typename C, typename T>
static uintptr_t GetOffset(T C::*member)
{
	T* ptr = &(reinterpret_cast<C*>(NULL)->*member);
	return reinterpret_cast<uintptr_t>(ptr);
}

DynamicCodeAllocator *FuncHooker::stubArea = NULL;
unsigned FuncHooker::instances = 0;

FuncHooker::FuncHooker(void *FunctionPtr, void *InjectionPtr) : installed(false),
					                                            stubCode(NULL),
					                                            InjectionFunc(InjectionPtr),
					                                            injectionJumpTarget(NULL),
					                                            overwriteSize(0),
					                                            backupCode(NULL),
					                                            proxyBackupCode(NULL),
					                                            proxyBackupCodeSize(0),
                                                                backupCodeSize(0),
					                                            hotpatchable(false),
					                                            disasm(new Disassembler(FunctionPtr)),
                                                                funcPtr((uint8_t*)FunctionPtr)
{
	if(!stubArea)
		stubArea = new DynamicCodeAllocator(sizeof(InjectionStub));

	++instances;

	try
	{
		FindFunctionBody();
	}
	catch(const std::exception& e)
	{
		delete disasm;
		throw e;
	}
}

FuncHooker::~FuncHooker()
{
	if(installed)
		RemoveHook();

	if(stubCode)
	{
		if(proxyBackupCode)
		{
			PrivelegeBlock write(injectionJumpTarget, proxyBackupCodeSize, PrivelegeBlock::ALL);
			std::memcpy(injectionJumpTarget, proxyBackupCode, proxyBackupCodeSize);
		}

		stubCode->~InjectionStub();
		stubArea->Free(stubCode);
	}

	delete [] proxyBackupCode;
	delete [] backupCode;

	delete disasm;

	if(!--instances)
	{
		delete stubArea;
		stubArea = NULL;
	}
}

bool FuncHooker::InstallHook()
{
	if(installed)
		return true;

	try
	{
		if(!stubCode)
		{
			if(!PrepareFunctionForHook())
				return false;
		}

		// Now we have to actually alter the original function. We're going to overwrite
		// the first few u8s with a jump to our injection function.
		// For starters, we're going to need to make the page writable
		PrivelegeBlock write(funcPtr, backupCodeSize, PrivelegeBlock::ALL);

		// Since multiple processes may be calling this function, it is probably a good
		// idea to do this as fast as possible.
		PriorityBlock fastest(100);

		// Pause all other threads while this write is happening
		SingleThreadBlock pauseThreads;

		// Make sure any thread IPs within the moved range are relocated to the stub
		pauseThreads.OffsetIPs(funcPtr, overwriteSize, stubCode);

		const uint8_t nop = 0x90;
		std::memset(funcPtr, nop, backupCodeSize);

#if defined(X64) || defined(WIN64)
		if(overwriteSize >= sizeof(ASM::LJmp))
			new (funcPtr) ASM::LJmp(injectionJumpTarget);
		else
#endif
		if(overwriteSize >= sizeof(ASM::Jmp))
			new (funcPtr) ASM::Jmp(funcPtr, injectionJumpTarget);
		else
			new (funcPtr) ASM::SJmp(funcPtr, injectionJumpTarget);

		installed = true;
		return true;
	}
	catch(const std::exception&)
	{
		return false;
	}
}

void FuncHooker::RemoveHook()
{
	if(!installed)
		return;

	assert(stubCode && "This function has not yet been hooked.");

	// Write the original backup code back into the function
	// For starters, we're going to need to make the page writable
	PrivelegeBlock write(funcPtr, backupCodeSize, PrivelegeBlock::ALL);

	// Since multiple processes may be calling this function, it is probably a good
	// idea to do this as fast as possible.
	PriorityBlock fastest(100);

	// Pause all other threads while this write is happening
	SingleThreadBlock pauseThreads;

	std::memcpy(funcPtr, backupCode, backupCodeSize);

	installed = false;
}

const void *FuncHooker::GetTrampoline() const
{
	return stubCode;
}

void FuncHooker::FindFunctionBody()
{
	// The function pointer may just be to a jump statement (IAT in
	// the case of a DLL). We want to fuck with the actual function,
	// so we're going to follow any jumps until we don't see a jump
	// which, by process of elimination, would hopefully mean we are
	// in the actual function.

	for(;;)
	{
		uint8_t *lastFuncPos = disasm->GetIP<uint8_t*>();
		Operation operation = disasm->ReadNextOperation();

		bool bodyFound = false;
		if(operation.GetMnemonic() == UD_Ijmp)
		{
			const Operand &operand = operation.GetOperand(0); // We know jump only has 1 operand

			// Move the current read position of the function to wherever the jump points to.
			// This works assuming our disassembler only disassembles one instruction at a time
			switch(operand.GetType())
			{
				// Jump to absolute address
				case UD_OP_PTR:{
					Operand::Ptr ptr = operand.GetValue<Operand::Ptr>();
					disasm->SetIP(reinterpret_cast<void*>((ptr.segment << 4) + ptr.offset));
				}break;
				// Jump to relative offset
				case UD_OP_JIMM:{
					uint8_t *curIP = disasm->GetIP<uint8_t*>();
					disasm->SetIP(curIP + operand.GetValue<int32_t>());
				}break;
				default:
					bodyFound = true;
				break;
			}
		}
		else
			 bodyFound = true;

		if(bodyFound)
		{
			disasm->SetIP(lastFuncPos);
			break;
		}
	}

	funcPtr = disasm->GetIP<uint8_t*>(); // Move our read pointer back one so we don't ignore an instruction
}


FuncHooker::DeadZone FuncHooker::FindNearestDeadZone(uint8_t *start, unsigned delta, unsigned minSize)
{
	uint8_t *prevIP = disasm->GetIP<uint8_t*>();

	// Lets get tricky. We want to overwrite as few u8s of the function as possible.
	// Unfortunately, a long jump is 5 u8s on x86 and on x64 could be 14 in the worst
	// case scenario. Solution: often in memory functions will be preceded by NOPs, INT 3's
	// (breakpoints), and other instructions which don't actually alter things in any way
	// (which makes sense, because the instruction pointer should never get there). A short
	// jump is only 2 u8s long. So, let's count the number of free u8s we have before
	// the function and, if it is enough, use a short jump to jump backward to our long jump
	// stored just before the function begins.

	// Unfortunately, disassemblers can't work in reverse, we'll have to simply look at u8s
	// one at a time and determine if they are a type of uint8_t we can view as a nop. Fortunately
	// all these NOP instruction types are only 1 uint8_t long.
	const uint8_t nop = 0x90;
	const uint8_t int3 = 0xCC;

	uint8_t *prefixFuncPtr = start-1;
	uint8_t *funcPageStart = PageManager::PageAlign(funcPtr); // Boundary of the page the function is on. Walking before this could cause an access violation

	// Time to count up our free u8s.
	while(prefixFuncPtr >= funcPageStart && (static_cast<uintptr_t>(funcPtr - prefixFuncPtr) <= minSize) &&
		  (*prefixFuncPtr == nop || *prefixFuncPtr == int3))
		--prefixFuncPtr;

	unsigned freePrefixu8s = static_cast<unsigned>(funcPtr - prefixFuncPtr - 1);
	if(freePrefixu8s >= minSize)
		return DeadZone(start - freePrefixu8s, freePrefixu8s);

	// Now to look forward to find any deadzones

	uint8_t *end = start + delta;
	disasm->SetIP(start);
	while(disasm->GetIP<uint8_t*>() < end)
	{
		Operation operation = disasm->ReadNextOperation();

		Operation::Mnemonic mnemonic = operation.GetMnemonic();
		if(mnemonic == UD_Inop || mnemonic == UD_Iint3)
		{
			DeadZone zone(disasm->GetIP<uint8_t*>() - 1); // -1 to not count the instruction we just read.

			while(disasm->GetIP<uint8_t*>() < end)
			{
				operation = disasm->ReadNextOperation();
				mnemonic = operation.GetMnemonic();

				if(mnemonic == UD_Inop || mnemonic == UD_Iint3)
					zone.len += operation.GetSize();
				else
					break;
			}

			if(zone.len >= minSize)
				return zone;
		}
	}

	disasm->SetIP(prevIP);
	return DeadZone(NULL, 0);
}

bool FuncHooker::PrepareFunctionForHook()
{
	// We need to do analysis to minimize the size of our alterations.

	// Allocate some memory for our trampoline (where we put the code from the
	// function we relocate). We try to make this as close to the function as
	// possible, because if it is within 2gb, we'll never need to find a deadzone
	// larger than 5 u8s as we'll be able to proxy through here as well
	uint8_t *stubMem = reinterpret_cast<uint8_t*>(stubArea->Allocate(funcPtr));

	// For starters, we need to find out exactly how far we'll need to jump
	// to get to our InjectionFunction from their function. If this distance
	// is greater than 2gb, we might need to use a 14 uint8_t 64 bit jump instead
	// of a 5 uint8_t regular jump. We want to minimize the number of u8s we're
	// overwritting with our jump to our InjectionFunction.
	intptr_t injectDist = reinterpret_cast<uint8_t*>(InjectionFunc) - funcPtr;
	intptr_t stubDist = stubMem - funcPtr;

	// Now we look for deadzones, or areas in code which are just NOPs or INT 3s.
	// If we can find one of these which is large enough within 127 u8s, we'll
	// be able to do a 2 uint8_t short jump to a proxy jump to our InjectionFunction.
	unsigned deadZoneMinSize;
#if defined(X64) || defined(WIN64)
	// We only need to find the full 14 u8s for a long jump if our InjectionFunction
	// is over 2gb away and our stub code is over 2gb away. Otherwise our proxy only
	// needs to be 5 u8s for a regular jump.
	if(std::abs(stubDist) > (1u<<31) - 1 && std::abs(injectDist) > (1u<<31) - 1)
		deadZoneMinSize = sizeof(ASM::LJmp);
	else
#endif
		deadZoneMinSize = sizeof(ASM::Jmp); // Otherwise, we just need 5 u8s for a regular jump.

	DeadZone deadZone = FindNearestDeadZone(funcPtr, 127, deadZoneMinSize);

	// If we found a deadzone, we can setup a proxy, yay!
	if(deadZone.addr)
	{
		overwriteSize = sizeof(ASM::SJmp); // We only need to erase 2 u8s for a short jump to our proxy
		injectionJumpTarget = deadZone.addr;

		InstallProxy(deadZone, reinterpret_cast<void*>(stubDist), reinterpret_cast<void*>(injectDist), stubMem, deadZoneMinSize);
	}
	else
	{
		// We couldn't write a 2u8 proxy. Determine our overwrite size.

		injectionJumpTarget = reinterpret_cast<uint8_t*>(InjectionFunc);
		overwriteSize = sizeof(ASM::Jmp);

#if defined(X64) || defined(WIN64)
		if(std::abs(injectDist) > (1u<<31) - 1)
		{
		    // We only need 14 u8s if both our stub code (for a long proxy) and our
		    // InjectionFunctiton are over 2gb away
			if(std::abs(stubDist) > (1u<<31) - 1)
				overwriteSize = sizeof(ASM::LJmp);
			else
				injectionJumpTarget = stubMem + GetOffset(&InjectionStub::executeInjector); // If our stub code is close, jump there instead.
		}
#endif
	}

	// Now actually allocate our stub code
	stubCode = new (stubMem) InjectionStub(funcPtr, InjectionFunc, overwriteSize);

	return RelocateFunctionHeader(overwriteSize);
}

void FuncHooker::InstallProxy(const DeadZone& deadZone, void *stubDistPtr, void *injectDistPtr, uint8_t *stubMem, unsigned deadZoneMinSize)
{
#if defined(X64) || defined(WIN64)
	// These are only needed in 64 bit for long jump checkings.
	intptr_t stubDist = reinterpret_cast<intptr_t>(stubDistPtr);
	intptr_t injectDist = reinterpret_cast<intptr_t>(injectDistPtr);
#else
	// Kill the warnings in 32 bit
	stubMem = stubMem; 
	stubDistPtr = stubDistPtr;
	injectDistPtr = injectDistPtr;
#endif

	// First, create a backup of the code we'll be overwriting with our proxy jump
	proxyBackupCodeSize = deadZoneMinSize;
	proxyBackupCode = new uint8_t[proxyBackupCodeSize];
	std::memcpy(proxyBackupCode, deadZone.addr, proxyBackupCodeSize);

	uint8_t *jumpTo;

#if defined(X64) || defined(WIN64)
	// If our stub is less than 2gb away and our InjectionFunc is more than 2gb
	// away, jump to the appropriate place in our stub instead.
	if(std::abs(stubDist) <= (1u<<31) - 1 && std::abs(injectDist) > (1u<<31) - 1)
		jumpTo = stubMem + GetOffset(&InjectionStub::executeInjector);
	else
#endif
		jumpTo = reinterpret_cast<uint8_t*>(InjectionFunc);

	// Now, we write in our jump.
	{
		// Make the page writeable first
		PrivelegeBlock write(deadZone.addr, proxyBackupCodeSize, PrivelegeBlock::ALL);

		if(proxyBackupCodeSize == sizeof(ASM::LJmp))
			new (deadZone.addr) ASM::LJmp(jumpTo);
		else
			new (deadZone.addr) ASM::Jmp(deadZone.addr, jumpTo);
	}
}

bool FuncHooker::RelocateFunctionHeader(unsigned headerSize)
{
	CodeRelocator relocator(funcPtr, stubCode->funcHeader, headerSize);

	disasm->SetIP(funcPtr);

	unsigned movedu8s = 0;
	while(movedu8s < headerSize)
	{
		Operation operation = disasm->ReadNextOperation();
		unsigned operSize = operation.GetSize();

		// If the whole header fits in 1 operation, we know we won't need to
		// pause threads while overwriting the function header as no thread
		// can possibly be in the middle of an operation.
		if(!movedu8s && operSize >= headerSize)
			hotpatchable = true;

		if(!relocator.Relocate(operation))
			return false;

		movedu8s += operation.GetSize();
	}

	// Make a backup copy of the area we are going to eventually overwrite.
	backupCodeSize = movedu8s;
	backupCode = new uint8_t[backupCodeSize];
	std::memcpy(backupCode, funcPtr, backupCodeSize);

	// Minor optimization. If we can skip all the nops in our function
	// header then be sure to do so.
	if(backupCodeSize + sizeof(ASM::SJmp) < sizeof(stubCode->funcHeader))
		new (stubCode->funcHeader + backupCodeSize) ASM::SJmp(static_cast<int8_t>(sizeof(stubCode->funcHeader) - backupCodeSize - sizeof(ASM::SJmp)));

	return true;
}

FuncHooker::DeadZone::DeadZone(uint8_t *addr, unsigned len) : addr(addr), len(len) {}
