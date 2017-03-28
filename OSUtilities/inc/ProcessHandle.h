/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		ProcessHandle.h
 *  \author		Andrew Shurney
 *  \brief		Manages a handle to a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef PROCESS_HANDLE_H
#define PROCESS_HANDLE_H

class ProcessHandle
{
	private:
		struct Data
		{
			void *handle;
			unsigned processId;
			unsigned rights;
			bool isX86; // this must default to true. x86 code works on x64 (almost always), but the opposite is true much less often.
		};

		Data *data;
		unsigned *refs;

		void Destroy();
		bool GrantRights(unsigned rights);

	public:
		ProcessHandle(unsigned procId);
		ProcessHandle(const ProcessHandle& rhs);
		ProcessHandle(ProcessHandle&& rhs);
		ProcessHandle& operator=(const ProcessHandle& rhs);
		ProcessHandle& operator=(ProcessHandle&& rhs);
		~ProcessHandle();

		bool EnsureRights(unsigned rights);

		bool IsX86() const;
		void SetX86(bool isX86);

		unsigned GetProcId() const;
		void *GetHandle() const;

		operator void*() const;
		operator unsigned() const;
};

#endif
