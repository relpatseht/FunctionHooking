/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		DynamicCodeAllocator.h
 *  \author		Andrew Shurney
 *  \brief		Allocates executable memory
 */

#ifndef DYNAMIC_CODE_ALLOCATOR_H
#define DYNAMIC_CODE_ALLOCATOR_H

#include <vector>
#include "PageManager.h"
#include "MemoryTree.h"

namespace std
{
	class memory_exception : public bad_alloc
	{
		protected:
			unsigned code;

		public:
			memory_exception(unsigned code);

			unsigned getCode() const;
	};
}

/*! \brief Allocator for sections of dynamically generated code.
*/
class DynamicCodeAllocator
{
	private:
		typedef std::vector<unsigned char*> PageList;

        unsigned size;           //!< Size (in bytes) of the allocation unit
		PageManager pageManager; //!< Manager pages of memory for the allocator.
		PageList pageList;       //!< All pages in use.
		MemoryTree freeTree;     //!< All free blocks of memory

		void CreatePage(void *addrNear) throw(std::memory_exception);

        DynamicCodeAllocator(const DynamicCodeAllocator&);            // Do not implement
        DynamicCodeAllocator& operator=(const DynamicCodeAllocator&); // Do not implement

	public:
		DynamicCodeAllocator(unsigned size);
		~DynamicCodeAllocator() throw();

		/// \brief Take an object from the free list and give it to the client (simulates new)
		/// \param[in] nearAddr - The returned address will be within +/- 2gb of this address.
        /// \exception memory_exception Thrown if the object can't be allocated. (Memory allocation problem)
		void *Allocate(void *nearAddr=NULL) throw(std::memory_exception);

        /// \brief Returns an object to the free list for the client (simulates delete)
        /// \exception memory_exception Thrown if the the object can't be freed. (Invalid object)
		void Free(void *object) throw(std::memory_exception);

        /// \brief Frees all empty pages
        unsigned FreeEmptyPages(void);
};

#endif
