/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SingleThreadBlock.cpp
 *  \author		Andrew Shurney
 *  \brief		Scoped object to pause all threads but the current
 */

#include "SingleThreadBlock.h"
#include "RemoteThread.h"
#include "RemoteThreadManager.h"
#include <cstdint>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

SingleThreadBlock::SingleThreadBlock(unsigned procId) : threadIds()
{
	while(PauseProcessThreads(procId));
}

SingleThreadBlock::~SingleThreadBlock()
{
	for(auto it=threadIds.begin(); it != threadIds.end(); ++it)
		it->second.Resume();
}

void SingleThreadBlock::OffsetIPs(void *start, unsigned range, void *dest)
{
	uint8_t *startPtr = reinterpret_cast<uint8_t*>(start);
	uint8_t *destPtr = reinterpret_cast<uint8_t*>(dest);

	for(auto it=threadIds.begin(); it != threadIds.end(); ++it)
	{
		const uint8_t *ipPtr = reinterpret_cast<const uint8_t*>(it->second.GetIp());

		if(ipPtr >= startPtr && ipPtr <= startPtr+range)
		{
			ipPtr = destPtr + (ipPtr - startPtr);
			it->second.SetIp(ipPtr);
		}
	}
}

unsigned SingleThreadBlock::PauseProcessThreads(unsigned procId)
{
	unsigned threadsPaused = 0;

#ifdef WIN32
	unsigned curThread = static_cast<unsigned>(GetCurrentThreadId());

	RemoteThreadManager remoteThreadManager(procId);
	RemoteThreadManager::RemoteThreads remoteThreads = remoteThreadManager.GetRemoteThreads();

	for(auto it = remoteThreads.begin(); it != remoteThreads.end(); ++it)
	{
		unsigned curThreadId = it->GetThreadId();
		if(curThreadId != curThread)
		{
			auto tIt = threadIds.find(curThreadId);
			if(tIt == threadIds.end())
			{
				it->Suspend();
				threadIds.insert(std::make_pair(curThreadId, *it));

				++threadsPaused;
			}
		}
	}
#else

#endif

	return threadsPaused;
}