/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SymbolFinderManager.h
 *  \author		Andrew Shurney
 *  \brief		Manages all active symbol finders
 */

#ifndef SYMBOL_FINDER_MANAGER_H
#define SYMBOL_FINDER_MANAGER_H

#include <unordered_map>

class SymbolFinder;

class SymbolFinderManager
{
	private:
		std::unordered_map<unsigned, SymbolFinder*> symbolFinders;

		SymbolFinderManager();
		~SymbolFinderManager();

	public:
		static const SymbolFinder* Get(unsigned procId = 0);
};

#endif
