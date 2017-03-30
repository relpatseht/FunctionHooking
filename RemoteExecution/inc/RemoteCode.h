/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteCode.h
 *  \author		Andrew Shurney
 *  \brief		Provides an interface to executing functions on another process
 */

#ifndef REMOTE_CODE_H
#define REMOTE_CODE_H

#include <string>
#include <unordered_map>
#include "RemoteVariable.h"
#include "RemoteFunction.h"

class ProcessHandle;
class CodeBuffer;
class RemoteThread;

#ifdef LoadLibrary
# undef LoadLibrary
#endif

/*! \brief Provides an interface to executing code on a different process

    <p>Example: If I wanted to call a function Foo in the module Test.dll 
	(which may or may not be loaded) on the process with id 1234, I would
	do something like this.</p>
	\code
	// Create the RemoteCode object on process with id 1234
	RemoteCode remoteCode(1234);
	if(!remoteCode.Initialize())
		return -1; // error

	// Retrieve a handle to the function Foo from the module Test.dll on the remote process
	RemoteFunction FooHandle = remoteCode.GetFunction("Test.dll", "Foo");
	if(!FooHandle.IsValid())
		return -1; // Error

	// Call the function Foo whose return type is uint32_t and takes 3 arguments
	// of type int, string, and double.
	uint32_t bar = FooHandle.Call<uint32_t>(4, "foo", 1.4);
	\endcode
*/
class RemoteCode
{
	private:
		typedef std::unordered_map<std::string, const void*> StringPtrMap;

		ProcessHandle *procHandle;
		CodeBuffer *buffer;
		RemoteThread *remoteThread;

		StringPtrMap remoteDllMap;
		StringPtrMap remoteFunctionMap;

		static std::string ToLower(std::string str);

		void PrepareRemoteThread();
		RemoteU32 InsertFinishedSignal(); // Returned variable set to 0 when execution completes.
		void InsertInfiniteLoop();
		RemoteU32 WriteCodeEpilogue();
		void WaitForFinish(RemoteU32& codeRunning);

		friend class RemoteFunction;
		void ResetRemoteCode();
		CodeBuffer& GetCodeBuffer();
		void Execute();

	public:
		/*! \brief Construct a remote code object on the process with the given id
		    \param[in] procId - Process id on which to create the proxy
		    
			Note: This constructor only creates the object. Initialize
			_must_ be called before use.

			\sa Initialize
		*/
		RemoteCode(unsigned procId);
		~RemoteCode();

		/*! \brief Get the id of the process this remote code object is interacting with.
		    \return process id
		*/
		unsigned GetProcId() const;

		/*! \brief Initialize the remote code object so it may be used.

		    This create a thread on the external process and grabs handles to the
			LoadLibrary and GetProcAddress functions it contains.

			\return True on success, false otherwise
		*/
		bool Initialize();

		/*! \brief Loads a module (dll) into the address space of the remote process
		    \param[in] dllAddr - Name (and path) of the dll to load.

			LoadLibrary will be called on the remote process with the argument provided.
			The address (a handle) to that library will be returned. DllMain will be called
			on the library. This method is added mostly for efficiency if you are loading 
			multiple functions from the same library.

			\return A handle to the remote dll.
		*/
		const void* LoadLibrary(const std::string& dllAddr);

		/* \brief Retrieves a function handle from a dll on the remote process.
		   \param[in] dll      - Name (and path) of the dll to load.
		   \param[in] function - Name of the function from the dll to load

		   If the dll or function is not already loaded, it will be. The returned
		   function handle is assumed to have a cdecl calling convention.

		   \return Handle to the remote function
		*/
		RemoteFunction GetFunction(const std::string& dll, const std::string& function);
		RemoteFunction GetFunction(const char* dll, const std::string& function);

		/* \brief Retrieves a function handle from a dll on the remote process.
		   \param[in] dllHandle - Handle (address) of the remote dll to use
		   \param[in] function  - Name of the function from the dll to load

		   If the dll or function is not already loaded, it will be. The returned
		   function handle is assumed to have a cdecl calling convention.

		   \return Handle to the remote function
		*/
		RemoteFunction GetFunction(const void* dllHandle, const std::string& function);
};

#endif
