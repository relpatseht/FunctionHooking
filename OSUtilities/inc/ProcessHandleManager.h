/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessHandleManager.h
 *  \author		Andrew Shurney
 *  \brief		Singleton for handling all process handles
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef PROCESS_HANDLE_MANAGER_H
#define PROCESS_HANDLE_MANAGER_H

#include <unordered_map>
#include "ProcessHandle.h"

class ProcessHandleManager
{
	private:
		typedef std::unordered_map<unsigned, ProcessHandle> ProcIdHandles;

		ProcIdHandles procIdHandles;

		ProcessHandleManager();
		~ProcessHandleManager();

	public:
		static ProcessHandleManager* Get();

		ProcessHandle GetHandle(unsigned procId);
};

#endif
