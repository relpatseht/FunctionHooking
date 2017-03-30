/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SymbolFinder.cpp
 *  \author		Andrew Shurney
 *  \brief		Finds symbols by searching module exports and the pdb
 */

#include "SymbolFinder.h"
#include "ProcessHandleManager.h"
#include "ModuleExplorer.h"
#include "Module.h"
#include <cstdint>
#include <algorithm>
#include <cctype>

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
# pragma warning(disable:4091)
# include <DbgHelp.h>
# pragma warning(default:4091)
#else

#endif

SymbolFinder::SymbolFinder(unsigned procId) : procHandle(ProcessHandleManager::Get()->GetHandle(procId)), addrToName(20000), nameToAddr(20000)
{
	if(!procHandle.EnsureRights(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION))
		throw std::exception();

	PopulateModuleExports();

#ifdef _MSC_VER
	if(!SymInitialize(procHandle, NULL, true))
		throw std::exception();
#else

#endif
}

SymbolFinder::~SymbolFinder()
{
     SymCleanup(procHandle);
}

const void *SymbolFinder::GetSymbolAddr(const char *symbol, const char* module) const
{
	static const char* kernel32Hack = "kernel32.dll";
	if(!module)
		module = kernel32Hack;

	auto itPair = nameToAddr.equal_range(symbol);
	if(itPair.first != nameToAddr.end())
	{
		if(module)
		{
			std::string modName = module;
			std::transform(modName.begin(), modName.end(), modName.begin(), std::tolower);

			for(auto it = itPair.first; it != itPair.second; ++it)
				if(it->second.first.GetName() == modName)
					return it->second.second;
		}

		return itPair.first->second.second;
	}

#ifdef _MSC_VER
	SYMBOL_INFO newStateSym;
	SecureZeroMemory(&newStateSym, sizeof(SYMBOL_INFO));
	newStateSym.SizeOfStruct = sizeof(SYMBOL_INFO);

	if(!SymFromName(procHandle, symbol, &newStateSym))
		return nullptr;

	return reinterpret_cast<void*>(newStateSym.Address);
#else

#endif
}

std::string SymbolFinder::GetSymbolName(const void *addr, const char* module) const
{
	auto itPair = addrToName.equal_range(addr);
	if(itPair.first != addrToName.end())
	{
		if(module)
		{
			std::string modName = module;
			std::transform(modName.begin(), modName.end(), modName.begin(), std::tolower);

			for(auto it=itPair.first; it != itPair.second; ++it)
				if(it->second.first.GetName() == modName)
					return it->second.second;
		}

		return itPair.first->second.second;
	}

#ifdef _MSC_VER
	const unsigned maxNameLen = 256;
	unsigned char buff[sizeof(SYMBOL_INFO) + (maxNameLen * sizeof(TCHAR))];

	SYMBOL_INFO *newStateSym = reinterpret_cast<SYMBOL_INFO*>(buff);
	SecureZeroMemory(newStateSym, sizeof(buff));
	newStateSym->SizeOfStruct = sizeof(SYMBOL_INFO);
	newStateSym->MaxNameLen = maxNameLen;

	if(!SymFromAddr(procHandle, reinterpret_cast<uintptr_t>(addr), NULL, newStateSym))
		return "";

	return std::string(newStateSym->Name, newStateSym->NameLen);
#else

#endif
}

void SymbolFinder::PopulateModuleExports()
{
	try
	{
		ModuleExplorer modExplorer(procHandle.GetProcId());
		ModuleExplorer::Modules modules = modExplorer.GetModules();

		for(auto it=modules.begin(); it != modules.end(); ++it)
		{
			try
			{
				Module::Functions functions = it->GetFunctions();
				for(auto fIt = functions.begin(); fIt != functions.end(); ++fIt)
				{
					addrToName.insert(std::make_pair(fIt->second, std::make_pair(*it, fIt->first)));
					nameToAddr.insert(std::make_pair(fIt->first, std::make_pair(*it, fIt->second)));
				}
			}
			catch(...)
			{
			}
		}
	}
	catch(...)
	{
	}
}
