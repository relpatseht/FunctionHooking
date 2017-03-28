/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteMemoryManager.h
 *  \author		Andrew Shurney
 *  \brief		Manages another processes memory
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef REMOTE_MEMORY_MANAGER_H
#define REMOTE_MEMORY_MANAGER_H

struct RemoteMemoryData;
class CodeBuffer;

class RemoteMemoryManager
{
	private:
		RemoteMemoryData *data;
		unsigned *refs;

		RemoteMemoryManager(unsigned procId);
		void Destroy();

	public:
		~RemoteMemoryManager();

		static RemoteMemoryManager Get(unsigned procId);

		RemoteMemoryManager(const RemoteMemoryManager& rhs);
		RemoteMemoryManager(RemoteMemoryManager&& rhs);
		RemoteMemoryManager& operator=(const RemoteMemoryManager& rhs);
		RemoteMemoryManager& operator=(RemoteMemoryManager&& rhs);

		void *Allocate(unsigned size);
		void Free(void *addr);

		CodeBuffer &GetCodeBuffer();

		template<typename T>
		T* Allocate()
		{
			return reinterpret_cast<T*>(Allocate(sizeof(T)));
		}
};

#endif
