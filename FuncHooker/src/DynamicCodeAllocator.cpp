/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		DynamicCodeAllocator.cpp
 *  \author		Andrew Shurney
 *  \brief		Allocates executable memory
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include <cassert>
#include <algorithm>
#include "PageManager.h"
#include "privateInc/DynamicCodeAllocator.h"

DynamicCodeAllocator::DynamicCodeAllocator(unsigned size) : size(std::max(size, MemoryTree::MinObjectSize())),
	                                                        pageManager(size), pageList(), freeTree(size)
{
}

DynamicCodeAllocator::~DynamicCodeAllocator() throw()
{
}

void *DynamicCodeAllocator::Allocate(void *nearAddr) throw(std::memory_exception)
{
	MemoryTree::GenericObject *obj = freeTree.GetRoot();
	MemoryTree::GenericObject **objParentPtr = NULL;

	if(nearAddr && sizeof(uintptr_t) > 4)
	{
		uint8_t *addr = reinterpret_cast<uint8_t*>(nearAddr);
		
		obj = freeTree.FindFirstInRange(addr - (1u<<31) - 1, addr + (1u<<31)-1, &objParentPtr);

		if(!obj)
		{
			CreatePage(nearAddr);
			obj = freeTree.FindFirstInRange(addr - (1u<<31) - 1, addr + (1u<<31)-1, &objParentPtr);
		}
	}

	if(!obj)
	{
		CreatePage(nearAddr);
		obj = freeTree.GetRoot();
	}

	freeTree.Erase(obj, objParentPtr);

	return obj;
}

void DynamicCodeAllocator::Free(void *addr) throw(std::memory_exception)
{
	freeTree.Insert(addr);
}

unsigned DynamicCodeAllocator::FreeEmptyPages()
{
	unsigned pagesFreed = 0;
	unsigned pageSize = pageManager.GetPageSize();
	unsigned objectsPerPage = pageSize / size;

	for(PageList::iterator it = pageList.begin(); it != pageList.end();)
	{
		uint8_t *pageStart = *it;
		uint8_t *pageEnd = pageStart + pageSize;

		MemoryTree::GenericObject **pageRootParentPtr; 
		MemoryTree::GenericObject *pageRoot = freeTree.FindFirstInRange(pageStart, pageEnd, &pageRootParentPtr);
		unsigned numFree = freeTree.CountInRange(pageRoot, pageStart, pageEnd);

        // All the page's objects are free
        if(numFree >= objectsPerPage)
        {
			freeTree.EraseRange(reinterpret_cast<MemoryTree::GenericObject*>(pageStart), 
				                reinterpret_cast<MemoryTree::GenericObject*>(pageStart + (objectsPerPage-1)*size), 
								pageRootParentPtr);

			pageManager.ReturnPage(pageStart);
			pageList.erase(it);
			++pagesFreed;
        }
        else
            ++it;
	}

	pageManager.ReleaseEmptyPages();

	return pagesFreed;
}

void DynamicCodeAllocator::CreatePage(void *addrNear) throw(std::memory_exception)
{
	unsigned objectsPerPage = pageManager.GetPageSize() / size;

	// Allocate a new page, throwing an FLException on failure
	unsigned char *pageMem = reinterpret_cast<unsigned char*>(pageManager.RequestPage(addrNear));
	if(!pageMem)
		throw std::memory_exception(/*"Out of physical memory.",*/ 0);

	pageManager.Commit(pageMem, pageManager.GetPageSize(), OSMemoryRights::READ | OSMemoryRights::WRITE | OSMemoryRights::EXECUTE);

	// Add the page to the pageList
	pageList.push_back(pageMem);

	// We want our object list relatively balanced. Since we know all insertions are going to be
	// consecutive, we can just insert them in a balanced fashion. Binary insertion.
	freeTree.BinaryInsert(pageMem, pageMem + objectsPerPage*size);
}

namespace std
{
	memory_exception::memory_exception(unsigned code) : bad_alloc(), code(code){}
	unsigned memory_exception::getCode() const { return code; }
}
