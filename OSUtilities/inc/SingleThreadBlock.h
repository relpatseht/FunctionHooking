/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		SingleThreadBlock.h
 *  \author		Andrew Shurney
 *  \brief		Scoped object to pause all threads but the current
 */

#ifndef SINGLE_THREAD_BLOCK_H
#define SINGLE_THREAD_BLOCK_H

#include <unordered_map>
#include "RemoteThread.h"

class SingleThreadBlock
{
	private:
		typedef std::unordered_map<unsigned, RemoteThread> ThreadIds;
		ThreadIds threadIds;

		unsigned PauseProcessThreads(unsigned procId);

	public:
		SingleThreadBlock(unsigned procId = 0);
		~SingleThreadBlock();

		void OffsetIPs(void *start, unsigned range, void *dest);
};

#endif
