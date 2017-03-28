/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PrivelegeBlock.cpp
 *  \author		Andrew Shurney
 *  \brief		Scoped memory privelege changer
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "PrivelegeBlock.h"
#include "OSMemoryRights.h"
#include <cstdint>

#ifdef _WIN32
# include <windows.h>
#else
# include "PageManager.h"
# include <sys/mman.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#endif

#ifndef WIN32

static unsigned GetNextNumber(const std::string& str, unsigned& start)
{
	while(!std::isdigit(str[start]) && start < str.length())
		start++;

	if(start >= str.length())
		return 0;

	unsigned num;
	std::istringstream sstr(str.substr(start));
	sstr >> num;

	return num;
}

static unsigned mpermissions(void *mem)
{
	uintptr_t page = reinterpret_cast<uintptr_t>(PageManager::PageAlign(mem));

	std::ifstream maps("/proc/self/maps", std::ios::in);
	if(!maps.is_open())
		return PROT_NONE;

	while(maps.good())
	{
		std::string line;
		std::getline(maps, line);

		unsigned numSearch = 0;
		unsigned memStart = GetNextNumber(line, numSearch);
		unsigned memEnd = GetNextNumber(line, ++numSearch);

		if(page >= memStart && page < memEnd)
		{
			unsigned permStart = numSearch + 1;

			while(!std::isprint(line[permStart]) && permStart < line.length())
				++permStart;

			if(permStart >= line.length())
				continue;

			unsigned permEnd = permStart;
			while(std::isprint(line[permEnd]) && permEnd < line.length())
				++permEnd;

			if(permEnd >= line.length())
				continue;

			unsigned permissions = PROT_NONE;
			std::string permStr = line.substr(permStart, permEnd - permStart);

			for(std::string::iterator it = permStr.begin(); it != permStr.end(); ++it)
			{
				switch(std::tolower(*it))
				{
					case 'r': permissions |= PROT_READ;  break;
					case 'w': permissions |= PROT_WRITE; break;
					case 'x': permissions |= PROT_EXEC;  break;
				}
			}

			return permissions;
		}
	}

	return PROT_NONE;
}


#endif

PrivelegeBlock::PrivelegeBlock(void *addr, unsigned size, unsigned privelege) : addr(addr), size(size), oldPrivelege()
{
#ifdef WIN32
	VirtualProtect(addr, size, OSMemoryRights::TranslateAccessToOS(privelege), (PDWORD)&oldPrivelege);
#else
	oldPrivelege = mpermissions(addr);

	uint8_t* addrFirstPage = PageManager::PageAlign(addr);
	uint8_t* addrLastPage = PageManager::PageAlign(reinterpret_cast<uint8_t*>(addr)+size);

	mprotect(addrFirstPage, PageManager::GetSysPageSize(), OSMemoryRights::TranslateAccessToOS(privelege));

	while(addrFirstPage != addrLastPage)
	{
		mprotect(addrFirstPage, PageManager::GetSysPageSize(), OSMemoryRights::TranslateAccessToOS(privelege));
		addrFirstPage += PageManager::GetSysPageSize();
	}

#endif
}

PrivelegeBlock::~PrivelegeBlock()
{
#ifdef _WIN32
	DWORD curPrivelege;
	VirtualProtect(addr, size, oldPrivelege, &curPrivelege);
#else
	uint8_t* addrFirstPage = PageManager::PageAlign(addr);
	uint8_t* addrLastPage = PageManager::PageAlign(reinterpret_cast<uint8_t*>(addr)+size);

	mprotect(addrFirstPage, PageManager::GetSysPageSize(), oldPrivelege);

	while(addrFirstPage != addrLastPage)
	{
		mprotect(addrFirstPage, PageManager::GetSysPageSize(), oldPrivelege);
		addrFirstPage += PageManager::GetSysPageSize();
	}
#endif
}
