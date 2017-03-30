/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		RemoteThreadManager.h
 *  \author		Andrew Shurney
 *  \brief		Grabs all threads on a process
 */

#ifndef REMOTE_THREAD_MANAGER
#define REMOTE_THREAD_MANAGER

#include <vector>
#include "RemoteThread.h"
#include "ProcessHandle.h"

class RemoteThreadManager
{
	public:
		typedef std::vector<RemoteThread> RemoteThreads;

	private:
		ProcessHandle procHandle;

	public:
		RemoteThreadManager(unsigned procId = 0);

		RemoteThreads GetRemoteThreads();
};

#endif
