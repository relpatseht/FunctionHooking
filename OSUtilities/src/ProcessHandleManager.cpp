/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessHandleManager.cpp
 *  \author		Andrew Shurney
 *  \brief		Singleton for handling all process handles
 */

#include "ProcessHandleManager.h"

ProcessHandleManager::ProcessHandleManager() : procIdHandles()
{

}

ProcessHandleManager::~ProcessHandleManager()
{

}

ProcessHandleManager* ProcessHandleManager::Get()
{
	static ProcessHandleManager manager;

	return &manager;
}

ProcessHandle ProcessHandleManager::GetHandle(unsigned procId)
{
	auto handleIt = procIdHandles.find(procId);
	if(handleIt == procIdHandles.end())
	{
		ProcessHandle handle(procId);
		handleIt = procIdHandles.insert(std::make_pair(procId, handle)).first;
	}
	
	return handleIt->second;
}
