/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessMemory.h
 *  \author		Andrew Shurney
 *  \brief		Read and writes to memory of a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef PROCESS_MEMORY_H
#define PROCESS_MEMORY_H

#include "ProcessHandle.h"

class ProcessMemory
{
	private:
		class Memory
		{
			public:
				virtual ~Memory() {}
				virtual void Write(void *addr, const void* val, unsigned size) = 0;
				virtual void Read(const void *addr, void* val, unsigned size) = 0;
		};

		class RemoteMemory : public Memory
		{
			private:
				ProcessHandle procHandle;

			public:
				RemoteMemory(unsigned procId);

				virtual void Write(void *addr, const void* val, unsigned size);
				virtual void Read(const void *addr, void* val, unsigned size);
		};

		class LocalMemory : public Memory
		{
			public:
				virtual void Write(void *addr, const void* val, unsigned size);
				virtual void Read(const void *addr, void* val, unsigned size);
		};

		Memory *memory;

		ProcessMemory(const ProcessMemory&);            // do not implement
		ProcessMemory& operator=(const ProcessMemory&); // do not implement

	public:
		ProcessMemory(unsigned procId=0);
		~ProcessMemory();
		
		void Write(void *addr, const void* val, unsigned size);
		void Read(const void *addr, void* val, unsigned size);

		template<typename T>
		void Write(void *addr, const T& val)
		{
			Write(addr, &val, sizeof(T));
		}

		template<typename T>
		void Read(const void *addr, T& val)
		{
			Read(addr, &val, sizeof(T));
		}

		template<typename T>
		T Read(const void *addr)
		{
			T val;
			Read(addr, val);

			return val;
		}
};

#endif
