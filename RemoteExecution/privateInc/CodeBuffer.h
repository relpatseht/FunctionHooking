/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		CodeBuffer.h
 *  \author		Andrew Shurney
 *  \brief		Manages a block of executable memory on a different process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef CODE_BUFFER_H
#define CODE_BUFFER_H

#include "ProcessHandle.h"
#include "ProcessMemory.h"

class PageManager;

class CodeBuffer
{
	private:
		PageManager& pageManager;
		ProcessHandle procHandle;
		unsigned char *mem;
		unsigned char *curPos;
		ProcessMemory memory;

		CodeBuffer(const CodeBuffer& rhs);            // do not implement
		CodeBuffer& operator=(const CodeBuffer& rhs); // do not implement

	public:
		enum
		{
			BEG,
			CUR,
			END
		};

		CodeBuffer(PageManager &pages);
		~CodeBuffer();

		template<typename T>
		bool Set(const T& val, unsigned num)
		{
			if(curPos + bytes >= mem + GetBufferSize())
				return false;

			bool ret = true;
			while(num-- && ret)
				ret &= Write(&val, sizeof(T));

			return ret;
		}

		bool Write(void *buffer, unsigned bytes);
		bool Seek(int offset, unsigned origin = CUR);
		unsigned Tell() const;
		const void *GetCurAddr() const;

		unsigned GetBufferSize() const;
};

#endif
