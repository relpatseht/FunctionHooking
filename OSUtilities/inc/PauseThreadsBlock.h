/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PauseThreadsBlock.h
 *  \author		Andrew Shurney
 *  \brief		Scoped object to pause all threads but the current
 */

/* All Content © 2012 Andrew Shurney, all rights reserved.              */

#ifndef SINGLE_THREAD_BLOCK_H
#define SINGLE_THREAD_BLOCK_H

#include <unordered_map>
#include "RemoteThread.h"

/*! \brief Pauses all threads in the specified process except the one which created the object.

    If running on a remote process, all threads will be paused. When the object
	goes out of scope, all threads will be removed. If the code has threading
	bugs, this object may expose them.
*/
class PauseThreadsBlock
{
	private:
		typedef std::unordered_map<unsigned, RemoteThread> ThreadIds;
		ThreadIds threadIds;

		unsigned PauseProcessThreads(unsigned procId);

	public:
		PauseThreadsBlock(unsigned procId = 0);
		~PauseThreadsBlock();

		/*! \brief Moves the instruction pointer of paused threads.
		    \param[in] start - Start address of the range
			\param[in] range - Distance, in bytes, to check from start.
			\param[in] dest - Where to put the instruction pointer.

			If the instruction pointer is in the range of [start, start+range),
			it will be moved to dest in all paused threads.
		*/
		void OffsetIPs(const void *start, unsigned range, const void *dest);
};

#endif
