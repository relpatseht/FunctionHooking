/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ModuleExplorer.h
 *  \author		Andrew Shurney
 *  \brief		Enumerates and manages modules in a process
 */

#ifndef MODULE_EXPLORER_H
#define MODULE_EXPLORER_H

#include "ProcessHandle.h"
#include "Module.h"
#include <vector>
#include <unordered_map>
#include <string>

class ModuleExplorer
{
	public:
		typedef std::vector<Module> Modules;

	private:
		typedef std::unordered_map<std::string, Module> ModuleMap;

		ProcessHandle procHandle;
		ModuleMap knownModules;

	public:
		ModuleExplorer(unsigned procId = 0);
		~ModuleExplorer();

		void Update();
		Modules GetModules(std::string nameStartsWith="");

		//void DeleteModule(void *addr) const;
		//void DeleteModule(const std::string& name) const;
};

#endif
