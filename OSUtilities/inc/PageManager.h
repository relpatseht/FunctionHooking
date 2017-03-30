/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PageManager.h
 *  \author		Andrew Shurney
 *  \brief		Manages pages of memory all platform independent like
 */

#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <vector>
#include <set>
#include <cstdint>
#include "OSMemoryRights.h"
#include "ProcessHandle.h"

/*! \brief Manages pages of memory, making requests directly to the os. */
class PageManager
{
	private:
		typedef std::vector<uint8_t*> Pages;
		typedef std::set<uint8_t*> SortedPages;
		typedef std::vector<Pages> VirtualPages;

		unsigned pageSize;           //!< Physical memory page size for the system
		unsigned virtualPageSize;    //!< Minimum allocation unit of virtual memory for the system.
		bool extraPage;              //!< If true, every page is twice page size.
		ProcessHandle procHandle;    //!< Handle to the process controlling pages of.

		VirtualPages pages;          //!< All pages held by the manager
		SortedPages freePages;       //!< All pages currently not commited.

		uint8_t *AllocatePage(void *addr);
		uint8_t *AllocatePageNear(void *near);
		void FreePage(void *addr);

	public:
		PageManager(unsigned pageSize, bool extraPage=false, unsigned procId = 0);
		~PageManager();

		/*! \brief Requests a page of memory.
		    \param[in] near - Address you want the returned page to be within +/- 2gb of.

			Near is only used when compiled in 64 bit mode.
		*/
		void* RequestPage(void *near=NULL);

		/*! \brief Returns a page to this page manager. */
		void ReturnPage(void *page);

		/*! \brief Return all empty pages of memory to the OS
		
			The entire virtual page needs to be uncommitted to release.
		*/
		void ReleaseEmptyPages();

		void Commit(void *mem, unsigned size, unsigned access = OSMemoryRights::READ | OSMemoryRights::WRITE);

		/*! \brief Changes the protection settings on an area of memory.

			\return Old protection access enum
		*/
		unsigned Protect(void *mem, unsigned size, unsigned access = OSMemoryRights::NO_ACCESS);

		unsigned GetPageSize() const;
		static unsigned GetSysPageSize();
		static uint8_t *PageAlign(void *addr);

		ProcessHandle GetProcHandle() const;
};

#endif