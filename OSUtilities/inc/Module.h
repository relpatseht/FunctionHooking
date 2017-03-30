/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		Module.h
 *  \author		Andrew Shurney
 *  \brief		Manages a single module in a process
 */

#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <unordered_map>
#include "ProcessHandle.h"

struct WIN_LDR_MODULE;

class Module
{
	public:
		typedef std::unordered_map<std::string, const void*> Functions;

	private:
		struct ModuleData
		{
			ProcessHandle procHandle;
			std::string name;
			void *address;
			unsigned size;

			ModuleData(unsigned procId);
		};

		ModuleData *data;
		unsigned *refs;

		void Destroy();

		friend class ModuleExplorer;
		Module(const WIN_LDR_MODULE& module, unsigned procId = 0);

	public:
		Module(const Module& rhs);
		Module(Module&& rhs);
		Module& operator=(const Module& rhs);
		Module& operator=(Module&& rhs);
		~Module();

		const std::string& GetName() const;
		const void *GetAddress() const;

		bool Contains(const void *addr) const;

		Functions GetFunctions() const;
};

#endif