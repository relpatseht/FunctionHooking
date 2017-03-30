/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessHandle.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a handle to a process
 */

#include "ProcessHandle.h"

#ifdef WIN32
# include <Windows.h>
#else
# error "Unsupported os"
#endif

ProcessHandle::ProcessHandle(unsigned procId) : data(new Data), refs(new unsigned)
{
	*refs = 1;

	data->rights = procId ? 0 : ~0u;
	data->isX86 = false;
#ifdef WIN32
	data->processId = procId ? procId : GetCurrentProcessId();
	data->handle = procId ? OpenProcess(data->rights, false, procId) : GetCurrentProcess();
#else

#endif
}

ProcessHandle::ProcessHandle(const ProcessHandle& rhs) : data(rhs.data), refs(rhs.refs)
{
	++(*refs);
}

ProcessHandle::ProcessHandle(ProcessHandle&& rhs) : data(rhs.data), refs(rhs.refs)
{
	rhs.data = nullptr;
	rhs.refs = nullptr;
}

ProcessHandle& ProcessHandle::operator=(const ProcessHandle& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

ProcessHandle& ProcessHandle::operator=(ProcessHandle&& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	rhs.data = nullptr;
	rhs.refs = nullptr;

	return *this;
}

ProcessHandle::~ProcessHandle()
{
	Destroy();
}

bool ProcessHandle::EnsureRights(unsigned rights)
{
	unsigned curRights = rights & data->rights;
	if(curRights == rights)
		return true;

	if(GrantRights(data->rights | rights))
		return true;

#ifdef WIN32
	// We couldn't get the rights we wanted, but if we can duplicate the handle,
	// we'll get full rights.
	if(GrantRights(PROCESS_DUP_HANDLE))
	{
		void *newHandle;
		if(DuplicateHandle(data->handle, data->handle, GetCurrentProcess(), &newHandle, 0, false, DUPLICATE_SAME_ACCESS))
		{
			CloseHandle(data->handle);
			data->handle = newHandle;
			data->rights = ~0u; // Full access now.

			return true;
		}
	}
#else

#endif

	return false;
}

bool ProcessHandle::IsX86() const      { return data->isX86;  }
void ProcessHandle::SetX86(bool isX86) { data->isX86 = isX86; }

unsigned ProcessHandle::GetProcId() const { return data->processId; }
void *ProcessHandle::GetHandle() const    { return data->handle;    }

ProcessHandle::operator void*() const
{
	return GetHandle();
}

ProcessHandle::operator unsigned() const
{
	return GetProcId();
}

void ProcessHandle::Destroy()
{
	if(!refs || !data)
		return;

	if(!(--(*refs)))
	{
		delete refs;

#ifdef WIN32
		CloseHandle(data->handle);
#else

#endif
		delete data;
	}
}

bool ProcessHandle::GrantRights(unsigned rights)
{
#ifdef WIN32
	void *newHandle = OpenProcess(rights, false, data->processId);
	if(newHandle)
	{
		CloseHandle(data->handle);
		data->handle = newHandle;
		data->rights = rights;
		return true;
	}
#else

#endif

	return false;
}
