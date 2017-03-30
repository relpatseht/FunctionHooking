/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		CodeBuffer.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a block of executable memory on a different process
 */

#include "privateInc/CodeBuffer.h"
#include "PageManager.h"
#include "OSMemoryRights.h"
#include <cstdint>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
#else
# error Unimplemented
#endif

CodeBuffer::CodeBuffer(PageManager &pages) : pageManager(pages), 
	                                         procHandle(pages.GetProcHandle()), 
	                                         mem(reinterpret_cast<uint8_t*>(pages.RequestPage())), 
											 curPos(mem), 
											 memory(procHandle.GetProcId())
{
#ifdef WIN32
	if(!procHandle.EnsureRights(PROCESS_VM_READ | PROCESS_VM_WRITE))
#else
#endif
		throw std::exception();

	pageManager.Commit(mem, pageManager.GetPageSize(), OSMemoryRights::READ | OSMemoryRights::WRITE | OSMemoryRights::EXECUTE);
}

CodeBuffer::~CodeBuffer()
{
	pageManager.ReturnPage(mem);
}

bool CodeBuffer::Write(void *buffer, unsigned bytes)
{
#ifdef WIN32
	if(!buffer || curPos + bytes >= mem + GetBufferSize())
		return false;

	memory.Write(curPos, buffer, bytes);
	if ( !FlushInstructionCache( procHandle.GetHandle(), curPos, bytes ) )
		return false;

	curPos += bytes;
#else
#endif

	return true;
}

bool CodeBuffer::Seek(int offset, unsigned origin)
{
	switch(origin)
	{
		case END:
			offset = GetBufferSize() - 1 + offset;
		case BEG:
			if(offset < 0 || offset >= static_cast<int>(GetBufferSize()))
				return false;
		break;
		case CUR:
			if(curPos + offset < mem || curPos + offset >= mem + GetBufferSize())
				return false;

			offset -= static_cast<int>(curPos - mem);
		break;
		default:
			return false;
	}

	curPos = mem + offset;
	return true;
}

unsigned CodeBuffer::Tell() const
{
	return static_cast<unsigned>(curPos - mem);
}

const void *CodeBuffer::GetCurAddr() const
{
	return curPos;
}

unsigned CodeBuffer::GetBufferSize() const
{
	return pageManager.GetPageSize();
}
