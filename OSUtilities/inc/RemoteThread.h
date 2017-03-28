/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		RemoteThread.h
 *  \author		Andrew Shurney
 *  \brief		Manages a thread on a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef REMOTE_THREAD_H
#define REMOTE_THREAD_H

#include "ProcessHandle.h"

struct THREAD_BASIC_INFORMATION;

class RemoteThread
{
	private:
		typedef long (__stdcall * NtQueryInformationThreadPtr)(void *threadHandle, 
			                                                   unsigned threadInformationClass,
														       void *threadInformation,
														       unsigned long threadInformationLength,
														       unsigned long *returnLength);
		static unsigned initialized;
		static void* ntdllHandle;
		static NtQueryInformationThreadPtr NtQueryInformationThread;

		void *threadHandle;
		unsigned *refs;

		void Destroy();
		static void GetThreadInfo(void *handle, THREAD_BASIC_INFORMATION& threadInfo);
		static void Initialize();
		static void Deinitialize();

	public:
		RemoteThread(unsigned threadId);
		RemoteThread(ProcessHandle procHandle, const void *startAddr, bool startSuspended = true);
		RemoteThread(const RemoteThread& rhs);
		RemoteThread(RemoteThread&& rhs);
		RemoteThread& operator=(const RemoteThread& rhs);
		RemoteThread& operator=(RemoteThread&& rhs);
		~RemoteThread();

		bool IsValid() const;

		void Suspend();
		void Resume();

		void WaitForDeath() const;
		unsigned GetThreadId() const;

		const void *GetIp() const;
		void SetIp(const void *ip);

		void* GetTEBAddr() const;

		static unsigned GetThreadId(void *handle);
};

#endif
