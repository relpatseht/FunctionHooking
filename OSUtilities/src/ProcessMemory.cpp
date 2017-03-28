/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessMemory.cpp
 *  \author		Andrew Shurney
 *  \brief		Read and writes to memory of a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "ProcessMemory.h"
#include "ProcessHandleManager.h"
#include <cstring>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
#else
# error unimplemented
#endif

ProcessMemory::ProcessMemory(unsigned procId) : memory(procId == GetCurrentProcessId() ? (Memory*)new LocalMemory() : (Memory*)new RemoteMemory(procId))
{
}

ProcessMemory::~ProcessMemory()
{
	delete memory;
}
		
void ProcessMemory::Write(void *addr, const void* val, unsigned size)
{
	memory->Write(addr, val, size);
}

void ProcessMemory::Read(const void *addr, void* val, unsigned size)
{
	memory->Read(addr, val, size);
}


ProcessMemory::RemoteMemory::RemoteMemory(unsigned procId) : procHandle(ProcessHandleManager::Get()->GetHandle(procId))
{
	if(!procHandle.EnsureRights(PROCESS_VM_WRITE | PROCESS_VM_READ))
		throw std::exception();
}

void ProcessMemory::RemoteMemory::Write(void *addr, const void* val, unsigned size)
{
	WriteProcessMemory(procHandle, addr, val, size, NULL);
}

void ProcessMemory::RemoteMemory::Read(const void *addr, void* val, unsigned size)
{
	ReadProcessMemory(procHandle, addr, val, size, NULL);
}

void ProcessMemory::LocalMemory::Write(void *addr, const void* val, unsigned size)
{
	std::memcpy(addr, val, size);
}

void ProcessMemory::LocalMemory::Read(const void *addr, void* val, unsigned size)
{
	std::memcpy(val, addr, size);
}
