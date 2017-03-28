/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SymbolFinderManager.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages all active symbol finders
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "SymbolFinderManager.h"
#include "SymbolFinder.h"

SymbolFinderManager::SymbolFinderManager() : symbolFinders() {}
SymbolFinderManager::~SymbolFinderManager() 
{
	for(auto it=symbolFinders.begin(); it != symbolFinders.end(); ++it)
		delete it->second;
}

const SymbolFinder* SymbolFinderManager::Get(unsigned procId)
{
	static SymbolFinderManager manager;

	auto it = manager.symbolFinders.find(procId);
	if(it == manager.symbolFinders.end())
		it = manager.symbolFinders.insert(std::make_pair(procId, new SymbolFinder(procId))).first;

	return it->second;
}
