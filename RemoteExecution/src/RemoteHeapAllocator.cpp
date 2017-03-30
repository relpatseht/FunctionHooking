/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteHeapAllocator.cpp
 *  \author		Andrew Shurney
 *  \brief		Heap allocator with a different process's memory
 */

#include "privateInc/RemoteHeapAllocator.h"
#include "PageManager.h"
#include <cstdint>
#include <algorithm>
#include <cassert>

static unsigned RoundUpSizeofUptr(unsigned x)
{
	return (x + (sizeof(uintptr_t) - 1)) & ~(sizeof(uintptr_t) - 1);
}

RemoteHeapAllocator::RemoteHeapAllocator(PageManager &pages) : pageManager(pages), pageList(), freeList(nullptr)
{
}

RemoteHeapAllocator::~RemoteHeapAllocator()
{
	Node *curNode = freeList;
	while(curNode)
	{
		Node *next = curNode->next;
		delete curNode;
		curNode = next;
	}
}

void *RemoteHeapAllocator::Allocate(unsigned size)
{
	unsigned actualSize = RoundUpSizeofUptr(size);

	if(actualSize > pageManager.GetPageSize() || !freeList && !AllocatePage())
		return nullptr;

	Node *prevNode = nullptr;
	Node *curNode = freeList;
	while(curNode)
	{
		void *addr = AllocateFromNode(curNode, prevNode, actualSize);
		if(addr)
			return addr;
		
		prevNode = curNode;
		curNode = curNode->next;
	}

	if(AllocatePage())
		return AllocateFromNode(freeList, nullptr, actualSize);

	return nullptr;
}

void RemoteHeapAllocator::Free(void *addrPtr)
{
	if(!addrPtr)
		return;

	uint8_t *mem = reinterpret_cast<uint8_t*>(addrPtr);
	auto allocIt = memInUse.find(mem);
	assert(allocIt != memInUse.end() && "Bad free. Never allocated.");

	unsigned size = allocIt->second;

	Node *prevNode = nullptr;
	Node *curNode = freeList;
	while(curNode)
	{
		uint8_t *nodeMem = reinterpret_cast<uint8_t*>(curNode->addr);
		uint8_t *nextMem = curNode->next ? reinterpret_cast<uint8_t*>(curNode->next->addr) : nullptr;

		if(mem+size == nodeMem)
		{
			curNode->size += size;
			curNode->addr = nodeMem - size;

			return;
		}

		if(nodeMem + curNode->size == mem)
		{
			curNode->size += size;

			Node *next = curNode->next;
			if(nodeMem + curNode->size == nextMem)
			{
				curNode->size += next->size; // nextMem would be null if next were null. We know next is valid
				curNode->next = next->next; 

				delete next;
			}

			return;
		}

		if(mem + size < nextMem)
		{
			Node *newNode = new Node(mem, size, curNode->next);
			curNode->next = newNode;
			return;
		}

		prevNode = curNode;
		curNode = curNode->next;
	}

	Node *newNode = new Node(mem, size);

	if(prevNode)
		prevNode->next = newNode;
	else
		freeList = newNode;
}
		
/// \brief Frees all empty pages
unsigned RemoteHeapAllocator::FreeEmptyPages(void)
{
	unsigned pagesFreed = 0;

	Node *prevNode = nullptr;
	Node *curNode = freeList;
	while(curNode)
	{
		auto nodeIt = std::find(pageList.begin(), pageList.end(), reinterpret_cast<uint8_t*>(curNode));
		if(nodeIt != pageList.end() && curNode->size >= pageManager.GetPageSize())
		{
			if(curNode->size > pageManager.GetPageSize())
			{
				Node *newNode = reinterpret_cast<Node*>(reinterpret_cast<uint8_t*>(curNode) + pageManager.GetPageSize());
				newNode->next = curNode->next;
				newNode->size = curNode->size - pageManager.GetPageSize();

				if(prevNode)
					prevNode->next = newNode;
				else
					freeList = newNode;
				
				curNode = newNode;
			}
			else
			{
				if(prevNode)
					prevNode->next = curNode->next;
				else
					freeList = curNode->next;

				curNode = curNode->next;
			}
			
			pageList.erase(nodeIt);
			pageManager.ReturnPage(curNode);
			++pagesFreed;
		}

		prevNode = curNode;
		curNode = curNode->next;
	}

	return pagesFreed;
}

bool RemoteHeapAllocator::AllocatePage()
{
	uint8_t *newPage = reinterpret_cast<uint8_t*>(pageManager.RequestPage());
	if(!newPage)
	{
		FreeEmptyPages();
		newPage = reinterpret_cast<uint8_t*>(pageManager.RequestPage());
		if(!newPage)
			return false;
	}
	pageManager.Commit(newPage, pageManager.GetPageSize(), OSMemoryRights::READ | OSMemoryRights::WRITE);

	pageList.push_back(newPage);

	Node *newNode = new Node(newPage, pageManager.GetPageSize());

	if(!freeList)
	{
		freeList = newNode;

		return true;
	}
	else
	{
		Node *curNode = freeList;
		while(curNode)
		{
			if(!curNode->next || newNode < curNode->next)
			{
				newNode->next = curNode->next;
				curNode->next = newNode;
				return true;
			}

			curNode = curNode->next;
		}
	}

	assert(false && "Unreachable code");
	delete newNode;
	pageManager.ReturnPage(newPage);
	pageList.pop_back();

	return false;
}

uint8_t* RemoteHeapAllocator::SplitNode(Node *node, unsigned size)
{
	uint8_t* mem = reinterpret_cast<uint8_t*>(node->addr);
	uint8_t *nodeMem = reinterpret_cast<uint8_t*>(node->addr);

	node->size -= size;
	node->addr = nodeMem + size;

	return mem;
}

void* RemoteHeapAllocator::AllocateFromNode(Node *node, Node *prevNode, unsigned size)
{
	uint8_t *mem = nullptr;
	if(node->size == size) 
	{
		if(prevNode)
			prevNode->next = node->next;
		else
			freeList = node->next;

		mem = reinterpret_cast<uint8_t*>(node->addr);

		delete node;

	}
	else if(node->size > size)
	{
		mem = SplitNode(node, size);
	}
	else
		return nullptr;

	memInUse.insert(std::make_pair(mem, size));

	return mem;
}

RemoteHeapAllocator::Node::Node(void *addr, unsigned size, Node *next) : next(next), addr(addr), size(size) {}