/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		RemoteThread.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a thread on a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "RemoteThread.h"
#include <cassert>
#include <cstdint>
#include "ProcessMemory.h"
#include <exception>
#include "UndocumentedStructs.h"

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#else
# error "Unimplemented"
#endif

unsigned RemoteThread::initialized = 0;
void *RemoteThread::ntdllHandle = nullptr;
RemoteThread::NtQueryInformationThreadPtr RemoteThread::NtQueryInformationThread = nullptr;

void RemoteThread::Initialize()
{
	if(!initialized++)
	{
		ntdllHandle = LoadLibrary("ntdll.dll");
		if(!ntdllHandle)
			throw std::exception("Couldn't load ntdll.dll. Check your permissions.");

		NtQueryInformationThread = (NtQueryInformationThreadPtr)GetProcAddress((HMODULE)ntdllHandle, "NtQueryInformationThread");

		if(!ntdllHandle || !NtQueryInformationThread)
			throw std::exception("Couldn't find NtQueryInformation thread.");
	}
}

void RemoteThread::Deinitialize()
{
	if(!--initialized)
	{
		FreeLibrary((HMODULE)ntdllHandle);
		ntdllHandle = nullptr;
		NtQueryInformationThread = nullptr;
	}
}

RemoteThread::RemoteThread(unsigned threadId) : threadHandle(OpenThread(THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, false, threadId)), refs(new unsigned)
{
	Initialize();

	assert(threadHandle && "Failed to open the remote thread. You don't have enough permissions.");

	*refs = 1;
}

RemoteThread::RemoteThread(ProcessHandle handle, const void* startAddr, bool startSuspended) : threadHandle(nullptr), refs(new unsigned)
{
	Initialize();

	if(!handle.EnsureRights(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ))
		assert(0 && "Could not get remote thread rights on the process.");

	threadHandle = CreateRemoteThread(handle, NULL, 0, reinterpret_cast<const LPTHREAD_START_ROUTINE>(startAddr), NULL, startSuspended ? CREATE_SUSPENDED : 0, NULL);

	assert(threadHandle && "Failed to create the remote thread.");

	*refs = 1;
}

RemoteThread::RemoteThread(const RemoteThread& rhs) : threadHandle(rhs.threadHandle), refs(rhs.refs)
{
	Initialize();
	++(*refs);
}

RemoteThread::RemoteThread(RemoteThread&& rhs) : threadHandle(rhs.threadHandle), refs(rhs.refs)
{
	Initialize();
	rhs.threadHandle = nullptr;
	rhs.refs = nullptr;
}

RemoteThread& RemoteThread::operator=(const RemoteThread& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	threadHandle = rhs.threadHandle;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

RemoteThread& RemoteThread::operator=(RemoteThread&& rhs)
{
	if(this == &rhs)
		return *this;

	threadHandle = rhs.threadHandle;
	refs = rhs.refs;

	rhs.threadHandle = nullptr;
	rhs.refs = nullptr;

	return *this;
}

RemoteThread::~RemoteThread()
{
	Destroy();
	Deinitialize();
}

void RemoteThread::Destroy()
{
	if(refs && !(--(*refs)))
	{
		delete refs;
		CloseHandle(threadHandle);
		
		refs = nullptr;
		threadHandle = nullptr;
	}
}

bool RemoteThread::IsValid() const
{
	return threadHandle != nullptr;
}

void RemoteThread::Suspend()
{
	SuspendThread(threadHandle);
}
void RemoteThread::Resume()
{
	ResumeThread(threadHandle);
}

void RemoteThread::WaitForDeath() const
{
	WaitForSingleObject(threadHandle, INFINITE);
}

unsigned RemoteThread::GetThreadId() const
{
	return RemoteThread::GetThreadId(threadHandle);
}

const void *RemoteThread::GetIp() const
{
	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_CONTROL;

	GetThreadContext(threadHandle, &threadContext);

	void *ipPtr = NULL;
# if defined(X64) || defined(WIN64)
	ipPtr = reinterpret_cast<void*>(threadContext.Rip);
# else
	ipPtr = reinterpret_cast<void*>(threadContext.Eip);
# endif

	return ipPtr;
}

void RemoteThread::SetIp(const void *ip)
{
	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_CONTROL;

	GetThreadContext(threadHandle, &threadContext);

# if defined(X64) || defined(WIN64)
	threadContext.Rip = reinterpret_cast<DWORD64>(ip);
# else
	threadContext.Eip = reinterpret_cast<DWORD>(ip);
# endif

	SetThreadContext(threadHandle, &threadContext);
}

void* RemoteThread::GetTEBAddr() const
{
	THREAD_BASIC_INFORMATION info;
	GetThreadInfo(threadHandle, info);

	return reinterpret_cast<void*>(info.TebBaseAddress);
}

void RemoteThread::GetThreadInfo(void *handle, THREAD_BASIC_INFORMATION& threadInfo)
{
	Initialize();

	unsigned status = NtQueryInformationThread(handle, 0, &threadInfo, sizeof(threadInfo), nullptr);
	assert(status == 0 && "Failed to query thread information.");

	Deinitialize();
}

unsigned RemoteThread::GetThreadId(void *handle)
{
	THREAD_BASIC_INFORMATION info;
	GetThreadInfo(handle, info);

	return reinterpret_cast<unsigned>(info.ClientId.UniqueThread);
}
