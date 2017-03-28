/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		PrioirtyBlock.cpp
 *  \author		Andrew Shurney
 *  \brief		Scoped thread priority changer
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "PriorityBlock.h"

#ifdef _WIN32
# include <Windows.h>
#else
# include <sys/time.h>
# include <sys/resource.h>
#endif

PriorityBlock::PriorityBlock(int newPriority) : oldPriority(), newPriority(newPriority)
{
#ifdef _WIN32
	HANDLE thread = GetCurrentThread();
	oldPriority = GetThreadPriority(thread);

	int priority;
	if     (newPriority <= -100) priority = THREAD_MODE_BACKGROUND_BEGIN;
	else if(newPriority <=  -50) priority = THREAD_PRIORITY_IDLE;
	else if(newPriority <=  -25) priority = THREAD_PRIORITY_LOWEST;
	else if(newPriority <=    0) priority = THREAD_PRIORITY_BELOW_NORMAL;
	else if(newPriority <=   50) priority = THREAD_PRIORITY_ABOVE_NORMAL;
	else if(newPriority <=   99) priority = THREAD_PRIORITY_HIGHEST;
	else                         priority = THREAD_PRIORITY_TIME_CRITICAL;

	SetThreadPriority(thread, priority);
#else
	oldPriority = getpriority(PRIO_PROCESS, 0);

	int priority = (-newPriority) * (20.0f/100.0f);
	if(priority < -20) priority = -20;
	else if(priority > 19) priority = 19;

	setpriority(PRIO_PROCESS, 0, priority);
#endif
}

PriorityBlock::~PriorityBlock()
{
#ifdef _WIN32
	HANDLE thread = GetCurrentThread();
	if(newPriority <= -100)
		SetThreadPriority(thread, THREAD_MODE_BACKGROUND_END);

	SetThreadPriority(thread, oldPriority);
#else
	setpriority(PRIO_PROCESS, 0, oldPriority);
#endif
}
