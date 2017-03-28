/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PageManager.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages pages of memory all platform independent like
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "PageManager.h"
#include "OSMemoryRights.h"
#include "ProcessHandleManager.h"
#include <algorithm>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/mman.h>
	#include <unistd.h>

	#ifndef MAP_UNINITIALIZED
		#define MAP_UNINITIALIZED 0x0
	#endif
#endif

static unsigned GetSystemPageSize()
{
#ifdef _WIN32
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwPageSize;
#else
	return sysconf(_SC_PAGE_SIZE);
#endif
}

PageManager::PageManager(unsigned size, bool extraPage, unsigned procId) : pageSize(0), virtualPageSize(0), 
	                                                                       extraPage(extraPage), 
																		   procHandle(ProcessHandleManager::Get()->GetHandle(procId)), 
																		   pages(), freePages()
{
	if(!procHandle.EnsureRights(PROCESS_VM_OPERATION))
		throw std::exception();

	unsigned sysPageSize = GetSysPageSize();
	unsigned sysVirtualPageSize = 64*1024;

	if(sysPageSize > size)
		pageSize = sysPageSize;
	else
		pageSize = size + (sysPageSize - (size % sysPageSize));

	if(sysVirtualPageSize > pageSize + (extraPage ? sysPageSize : 0))
		virtualPageSize = sysVirtualPageSize;
	else
	{
		size = pageSize + (extraPage ? sysPageSize : 0);
		virtualPageSize = size + (sysVirtualPageSize - (size % sysVirtualPageSize));
	}
}

PageManager::~PageManager()
{
	for(VirtualPages::iterator vIt=pages.begin(); vIt != pages.end(); ++vIt)
		FreePage(vIt->front());
}

void* PageManager::RequestPage(void *nearAddr)
{
	if(freePages.empty())
	{
		uint8_t *mem = AllocatePageNear(nearAddr);

		if(!mem)
			return NULL;

		pages.push_back(Pages());
		Pages& newPages = pages.back();
		uint8_t *memEnd = mem + virtualPageSize;
		unsigned memIncrement = extraPage ? pageSize + GetSysPageSize() : pageSize;

		for(; static_cast<unsigned>(memEnd - mem) >= memIncrement ; mem += memIncrement)
		{
			freePages.insert(mem);
			newPages.push_back(mem);
		}
	}

	SortedPages::iterator freePageIt = freePages.end();
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)
#endif
	if(sizeof(uintptr_t) > 4 && nearAddr)
#ifdef _MSC_VER
# pragma warning(pop)
#endif
		freePageIt = freePages.lower_bound(reinterpret_cast<uint8_t*>(nearAddr));

	if(freePageIt == freePages.end())
		freePageIt = freePages.begin();

	uint8_t *page = *freePageIt;
	freePages.erase(freePageIt);

	return page;
}

void PageManager::ReturnPage(void *page)
{
	freePages.insert(reinterpret_cast<uint8_t*>(page));

#ifdef _WIN32
	VirtualAllocEx(procHandle, page, pageSize, MEM_RESERVE, PAGE_NOACCESS);
#else
	mmap(page, pageSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0);
#endif
}

void PageManager::ReleaseEmptyPages()
{
	for(VirtualPages::iterator vIt = pages.begin(); vIt != pages.end();)
	{
		bool commitedPage = false;
		for(Pages::iterator it = vIt->begin(); it != vIt->end() && !commitedPage; ++it)
			if(std::find(freePages.begin(), freePages.end(), *it) == freePages.end())
				commitedPage = true;

		if(!commitedPage)
		{
			for(Pages::iterator it = vIt->begin(); it != vIt->end() && !commitedPage; ++it)
				freePages.erase(std::find(freePages.begin(), freePages.end(), *it));

			FreePage(vIt->front());

			vIt = pages.erase(vIt);
		}
		else
			++vIt;
	}
}

void PageManager::Commit(void *mem, unsigned size, unsigned access)
{
#ifdef WIN32
	VirtualAllocEx(procHandle, mem, size, MEM_COMMIT, OSMemoryRights::TranslateAccessToOS(access));
#else
	mprotect(mem, size, OSMemoryRights::TranslateAccessToOS(access));
#endif
}

unsigned PageManager::Protect(void *mem, unsigned size, unsigned access)
{
	unsigned oldAccess = 0;
#ifdef _WIN32
	DWORD win32OldAccess;
	VirtualProtectEx(procHandle, mem, size, OSMemoryRights::TranslateAccessToOS(access), &win32OldAccess);
	oldAccess = win32OldAccess;
#else
	mprotect(mem, size, OSMemoryRights::TranslateAccessToOS(access));
#endif

	return OSMemoryRights::TranslateAccessFromOS(oldAccess);
}

unsigned PageManager::GetPageSize() const
{
	return pageSize;
}

unsigned PageManager::GetSysPageSize()
{
	static unsigned sysPageSize = GetSystemPageSize();
	return sysPageSize;
}

uint8_t *PageManager::PageAlign(void *addr)
{
	uintptr_t addrVal = reinterpret_cast<uintptr_t>(addr);
	addrVal &= ~(static_cast<uintptr_t>(GetSysPageSize()-1));

	return reinterpret_cast<uint8_t*>(addrVal);
}

uint8_t *PageManager::AllocatePageNear(void *nearVoid)
{
	uint8_t *nearPage = PageManager::PageAlign(nearVoid);

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)
#endif
	if(sizeof(uintptr_t) > 4 && nearVoid)
#ifdef _MSC_VER
# pragma warning(pop)
#endif
	{
		const unsigned maxEffort = 100;
		unsigned attempts = 0;
		unsigned offset = virtualPageSize;
		do
		{
			uint8_t *mem = AllocatePage(nearPage + offset);
			if(mem)
			{
				if(std::abs(nearPage - mem) < (1u<<31) - 1)
					return mem;
				else
					FreePage(mem);
			}

			offset += virtualPageSize;
		} while(attempts++ < maxEffort);
	}

	return AllocatePage(NULL);
}

uint8_t *PageManager::AllocatePage(void *addr)
{
#ifdef WIN32
	return reinterpret_cast<uint8_t*>(VirtualAllocEx(procHandle, addr, virtualPageSize, MEM_RESERVE, PAGE_NOACCESS));
#else
	return reinterpret_cast<uint8_t*>(mmap(addr, virtualPageSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0));
#endif
}

void PageManager::FreePage(void *addr)
{
#ifdef WIN32
	VirtualFreeEx(procHandle, addr, 0, MEM_RELEASE);
#else
	munmap(addr, virtualPageSize);
#endif
}

ProcessHandle PageManager::GetProcHandle() const
{
	return procHandle;
}
