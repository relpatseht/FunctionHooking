/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteMemoryManager.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages another processes memory
 */

#include "privateInc/RemoteMemoryManager.h"
#include <unordered_map>
#include "PageManager.h"
#include "privateInc/RemoteHeapAllocator.h"
#include "privateInc/CodeBuffer.h"

struct RemoteMemoryData
{
	PageManager   pageManager;
	CodeBuffer    code;
	RemoteHeapAllocator heap;

	RemoteMemoryData(unsigned procId) : pageManager(4*1024, false, procId), code(pageManager), heap(pageManager){}
};

RemoteMemoryManager RemoteMemoryManager::Get(unsigned procId)
{
	typedef std::unordered_map<unsigned, RemoteMemoryManager> ProcMemoryMap;
	static ProcMemoryMap procMemory;

	auto memIt = procMemory.find(procId);
	if(memIt == procMemory.end())
		memIt = procMemory.insert(std::make_pair(procId, RemoteMemoryManager(procId))).first;

	return memIt->second;
}

RemoteMemoryManager::RemoteMemoryManager(unsigned procId) : data(new RemoteMemoryData(procId)), refs(new unsigned)
{
	*refs = 1;
}

RemoteMemoryManager::RemoteMemoryManager(const RemoteMemoryManager& rhs) : data(rhs.data), refs(rhs.refs)
{
	++(*refs);
}

RemoteMemoryManager::RemoteMemoryManager(RemoteMemoryManager&& rhs) : data(rhs.data), refs(rhs.refs)
{
	rhs.data = nullptr;
	rhs.refs = nullptr;
}

RemoteMemoryManager& RemoteMemoryManager::operator=(const RemoteMemoryManager& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

RemoteMemoryManager& RemoteMemoryManager::operator=(RemoteMemoryManager&& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	rhs.data = nullptr;
	rhs.refs = nullptr;

	return *this;
}

RemoteMemoryManager::~RemoteMemoryManager()
{
	Destroy();
}

void RemoteMemoryManager::Destroy()
{
	if(refs && data && !(--(*refs)))
	{
		delete refs;
		delete data;
		refs = nullptr;
		data = nullptr;
	}
}

void *RemoteMemoryManager::Allocate(unsigned size)
{
	return data->heap.Allocate(size);
}

void RemoteMemoryManager::Free(void *addr)
{
	data->heap.Free(addr);
}

CodeBuffer &RemoteMemoryManager::GetCodeBuffer()
{
	return data->code;
}
