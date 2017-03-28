/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ModuleExplorer.cpp
 *  \author		Andrew Shurney
 *  \brief		Enumerates and manages modules in a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "ModuleExplorer.h"
#include "ProcessHandleManager.h"
#include "RemoteThreadManager.h"
#include <cstdint>
#include "ProcessMemory.h"
#include "UndocumentedStructs.h"
#include <cctype>
#include <algorithm>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
# include <Winternl.h>
#else
# error unimplemented
#endif

ModuleExplorer::ModuleExplorer(unsigned procId) : procHandle(ProcessHandleManager::Get()->GetHandle(procId))
{
}

ModuleExplorer::~ModuleExplorer()
{
}

void ModuleExplorer::Update()
{
	ProcessMemory memory(procHandle.GetProcId());

	RemoteThreadManager::RemoteThreads threads = RemoteThreadManager(procHandle.GetProcId()).GetRemoteThreads();

	// We need to grab a thread from the process, and it can't be the one we are running on
	// because we need to suspend it in order to get it's teb
	RemoteThread thread(threads[0]);

	if(thread.GetThreadId() == ::GetCurrentThreadId())
		thread = threads.at(1);

	thread.Suspend();
	void *tebAddr = thread.GetTEBAddr();
	thread.Resume();

	TEB teb = memory.Read<TEB>(tebAddr);
	PEB peb = memory.Read<PEB>(teb.ProcessEnvironmentBlock);
	WIN_PEB_LDR_DATA ldrData = memory.Read<WIN_PEB_LDR_DATA>(peb.Ldr);

	WIN_LDR_MODULE curModule = memory.Read<WIN_LDR_MODULE>(ldrData.InLoadOrderModuleList.Flink);

	while(curModule.BaseAddress)
	{
		Module mod(curModule, procHandle.GetProcId());

		std::string lowerModName = mod.GetName();
		std::transform(lowerModName.begin(), lowerModName.end(), lowerModName.begin(), std::tolower);
		knownModules.insert(std::make_pair(lowerModName, mod));

		memory.Read(curModule.InLoadOrderModuleList.Flink, curModule);
	}
}

ModuleExplorer::Modules ModuleExplorer::GetModules(std::string nameStartsWith)
{
	Modules modules;

	std::transform(nameStartsWith.begin(), nameStartsWith.end(), nameStartsWith.begin(), std::tolower);
	for(auto it = knownModules.begin(); it != knownModules.end(); ++it)
		if(!it->first.compare(0, nameStartsWith.size(), nameStartsWith)) 
			modules.push_back(it->second);

	if(!modules.size())
	{
		Update();
		for(auto it = knownModules.begin(); it != knownModules.end(); ++it)
			if(!it->first.compare(0, nameStartsWith.size(), nameStartsWith)) 
				modules.push_back(it->second);
	}

	return modules;
}
