/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		RemoteThreadManager.cpp
 *  \author		Andrew Shurney
 *  \brief		Grabs all threads on a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "RemoteThreadManager.h"
#include "ProcessHandleManager.h"

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <Tlhelp32.h>
#else
# error "Unimplemented"
#endif

RemoteThreadManager::RemoteThreadManager(unsigned procId) : procHandle(ProcessHandleManager::Get()->GetHandle(procId))
{}

RemoteThreadManager::RemoteThreads RemoteThreadManager::GetRemoteThreads()
{
	RemoteThreads threads;

	DWORD procId = procHandle.GetProcId();

	HANDLE threadsHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(threadsHandle != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 threadEntry;
		threadEntry.dwSize = sizeof(threadEntry);

		if(Thread32First(threadsHandle, &threadEntry))
		{
			do 
			{
				if((threadEntry.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(threadEntry.th32OwnerProcessID)) &&
				   (procId == threadEntry.th32OwnerProcessID))
					threads.push_back(RemoteThread(threadEntry.th32ThreadID));

				threadEntry.dwSize = sizeof(threadEntry);
			} while (Thread32Next(threadsHandle, &threadEntry));
		}

		CloseHandle(threadsHandle);
	}

	return threads;
}
