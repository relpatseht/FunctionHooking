/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteHeapAllocator.h
 *  \author		Andrew Shurney
 *  \brief		Heap allocator with a different process's memory
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef HEAP_ALLOCATOR_H
#define HEAP_ALLOCATOR_H

#include <vector>
#include <unordered_map>
#include <cstdint>
class PageManager;

class RemoteHeapAllocator
{
	private:
		typedef std::vector<unsigned char*> PageList;
		typedef std::unordered_map<uint8_t*, unsigned> Allocations;

		struct Node
		{
			Node *next;
			void *addr;
			unsigned size;

			Node(void *addr = nullptr, unsigned size = 0, Node *next = nullptr);
		};

		PageManager &pageManager; //!< Manager pages of memory for the allocator.
		PageList pageList;        //!< All pages in use.
		Node *freeList;           //!< List of free memory blocks.
		Allocations memInUse;     //!< The size of all allocated blocks

		RemoteHeapAllocator(const RemoteHeapAllocator&);            // do not implement
		RemoteHeapAllocator& operator=(const RemoteHeapAllocator&); // do not implement

		bool AllocatePage();
		void* AllocateFromNode(Node *node, Node *prevNode, unsigned size);
		uint8_t* SplitNode(Node *node, unsigned size);

	public:
		RemoteHeapAllocator(PageManager &pages);
		~RemoteHeapAllocator();

		void *Allocate(unsigned size);
		void Free(void *addr);
		
        /// \brief Frees all empty pages
        unsigned FreeEmptyPages(void);

		template<typename T>
		T* Allocate()
		{
			return reinterpret_cast<T*>(Allocate(sizeof(T)));
		}
};

#endif
