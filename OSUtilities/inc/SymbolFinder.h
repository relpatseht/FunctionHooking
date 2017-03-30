/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SymbolFinder.h
 *  \author		Andrew Shurney
 *  \brief		Finds symbols by searching module exports and the pdb
 */

#ifndef SYMBOL_FINDER_H
#define SYMBOL_FINDER_H

#include <string>
#include <unordered_map>
#include "ProcessHandle.h"
#include "Module.h"

class SymbolFinder
{
	private:
		ProcessHandle procHandle;
		std::unordered_multimap<const void*, std::pair<Module, std::string> > addrToName;
		std::unordered_multimap<std::string, std::pair<Module, const void*> > nameToAddr;

		void PopulateModuleExports();
		
		SymbolFinder(unsigned procId = 0);
		~SymbolFinder();

		friend class SymbolFinderManager;

	public:
		const void *GetSymbolAddr(const char* symbol, const char* module=nullptr) const;
		std::string GetSymbolName(const void *addr, const char* module=nullptr) const;
};

#endif
