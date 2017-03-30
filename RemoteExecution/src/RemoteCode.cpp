/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteCode.cpp
 *  \author		Andrew Shurney
 *  \brief		Provides an interface to executing functions on another process
 */

#include "RemoteCode.h"
#include "ASMStubs.h"
#include "ProcessHandle.h"
#include "ProcessHandleManager.h"
#include "privateInc/CodeBuffer.h"
#include "RemoteThread.h"
#include "privateInc/RemoteMemoryManager.h"
#include <cstdint>
#include "ModuleExplorer.h"
#include "Module.h"
#include <cctype>
#include <algorithm>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# ifdef LoadLibrary
#  undef LoadLibrary
# endif
#endif

RemoteCode::RemoteCode(unsigned procId) : procHandle(new ProcessHandle(ProcessHandleManager::Get()->GetHandle(procId))),
	                                      buffer(&RemoteMemoryManager::Get(procId).GetCodeBuffer()),
										  remoteThread(nullptr),
										  remoteDllMap(), remoteFunctionMap()
{
}

RemoteCode::~RemoteCode()
{
	delete procHandle;

	if(remoteThread)
	{
		ResetRemoteCode();

		// Kill the remote thread by returning.
		ASM::Return ret;
		buffer->Write(&ret, sizeof(ret));
		remoteThread->Resume(); // Start executing

		remoteThread->WaitForDeath();

		delete remoteThread;
	}
}

unsigned RemoteCode::GetProcId() const
{
	return procHandle->GetProcId();
}

bool RemoteCode::Initialize()
{
	if(remoteThread)
		return true;

	PrepareRemoteThread();
	
	ModuleExplorer modExp(GetProcId());
	ModuleExplorer::Modules kernel32s = modExp.GetModules("kernel32.dll");
	if(!kernel32s.size())
		return false;

	Module& kernel32 = kernel32s[0];

	const void *kernel32Module = kernel32.GetAddress();
	if(!kernel32Module)
		return false;

	remoteFunctionMap = kernel32.GetFunctions();

	const void *LoadLibraryAddr = remoteFunctionMap["LoadLibraryA"];
	const void *GetProcAddressAddr = remoteFunctionMap["GetProcAddress"];

	if(!remoteThread->IsValid() || !kernel32Module || !LoadLibraryAddr || ! GetProcAddressAddr)
	{
		remoteFunctionMap.clear();
		delete remoteThread;
		return false;
	}

	remoteDllMap["kernel32.dll"] = kernel32Module;

	return true;
}

const void* RemoteCode::LoadLibrary(const std::string& dllAddr)
{
	auto loadLibIt = remoteFunctionMap.find("LoadLibraryA");
	if(loadLibIt == remoteFunctionMap.end())
		return nullptr;

	RemoteStr dllName(procHandle->GetProcId(), dllAddr);
	RemoteUPtr libraryModule(procHandle->GetProcId());

	RemoteFunction remoteLoadLibrary(*this);
	remoteLoadLibrary.SetAddr(loadLibIt->second);
	remoteLoadLibrary.SetCallingConvention(RemoteFunction::CC_STDCALL);

	remoteLoadLibrary.CallV(RemoteFunction::Arguments(1, std::addressof(dllName)), std::addressof(libraryModule));

	void *libaryModuleHandle = reinterpret_cast<void*>((uintptr_t)libraryModule);

	if(libaryModuleHandle)
		remoteDllMap.insert(std::make_pair(dllAddr, libaryModuleHandle));

	return libaryModuleHandle;
}

RemoteFunction RemoteCode::GetFunction(const char* dll, const std::string& function)
{
	return GetFunction(std::string(dll), function);
}

RemoteFunction RemoteCode::GetFunction(const std::string& dll, const std::string& function)
{
	auto libraryIt = remoteDllMap.find(ToLower(dll));
	const void *libraryHandle = nullptr;

	if(libraryIt == remoteDllMap.end())
		libraryHandle = LoadLibrary(dll);
	else
		libraryHandle = libraryIt->second;

	if(!libraryHandle)
		return RemoteFunction(*this);

	return GetFunction(libraryHandle, function);
}

RemoteFunction RemoteCode::GetFunction(const void* dllHandle, const std::string& function)
{
	auto getProcIt = remoteFunctionMap.find("GetProcAddress");
	if(getProcIt == remoteFunctionMap.end())
		return RemoteFunction(*this);

	RemoteUPtr libraryModule(procHandle->GetProcId(), reinterpret_cast<uintptr_t>(dllHandle));
	RemoteStr funcName(procHandle->GetProcId(), function);
	RemoteUPtr funcAddr(procHandle->GetProcId());

	RemoteFunction remoteGetProc(*this);
	remoteGetProc.SetAddr(getProcIt->second);
	remoteGetProc.SetCallingConvention(RemoteFunction::CC_STDCALL);

	RemoteFunction::Arguments args;
	args.push_back(std::addressof(libraryModule));
	args.push_back(std::addressof(funcName));

	remoteGetProc.CallV(args, std::addressof(funcAddr));

	void *funcAddrPtr = reinterpret_cast<void*>((uintptr_t)funcAddr);
	if(funcAddrPtr)
		remoteFunctionMap.insert(std::make_pair(function, funcAddrPtr));

	RemoteFunction retFunc(*this);
	retFunc.SetAddr(funcAddrPtr);

	return retFunc;
}


void RemoteCode::ResetRemoteCode()
{
	buffer->Seek(0, CodeBuffer::BEG);
	remoteThread->SetIp(buffer->GetCurAddr());
}

CodeBuffer& RemoteCode::GetCodeBuffer()
{
	return *buffer;
}

void RemoteCode::Execute()
{
	RemoteU32 codeRunning = WriteCodeEpilogue();

	// But the buffer back to the beginning and set our ip to there
	ResetRemoteCode();

	remoteThread->Resume(); // Start executing

	WaitForFinish(codeRunning);
}


std::string RemoteCode::ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::tolower);
	return str;
}

void RemoteCode::PrepareRemoteThread()
{
	buffer->Seek(0, CodeBuffer::BEG);
	const void *bufferStart = buffer->GetCurAddr();

	ASM::IsX86 x86Test;
	RemoteU32 isX86(procHandle->GetProcId());
	isX86 = 1;

	buffer->Write(&x86Test, sizeof(x86Test)); // Determine if the exe is x86 or x64
	isX86.GetReturnVal(*buffer);

	RemoteU32 codeRunning = WriteCodeEpilogue(); // Windows is going to execute some basic bookkeeping.

	remoteThread = new RemoteThread(*procHandle, bufferStart, false);

	WaitForFinish(codeRunning);

	procHandle->SetX86(((uint32_t)isX86) != 0); // Save exe's status.

	ResetRemoteCode();
}

RemoteU32 RemoteCode::InsertFinishedSignal()
{
	RemoteU32 codeRunning(procHandle->GetProcId());
	codeRunning = 1;
	ASM::MovToReg clearEAX(0, ASM::REG::EAX);

	// Set things up so codeRunning will be set to 0 when the thread has finished execution
	buffer->Write(&clearEAX, sizeof(clearEAX));
	codeRunning.GetReturnVal(*buffer);

	return codeRunning;
}

void RemoteCode::InsertInfiniteLoop()
{
	// Add an infinite loop to the end so we never start executing random memory
	ASM::SJmp infiniteLoop(-static_cast<int8_t>(sizeof(ASM::SJmp)));
	buffer->Write(&infiniteLoop, sizeof(infiniteLoop));
}

RemoteU32 RemoteCode::WriteCodeEpilogue()
{
	RemoteU32 codeRunning = InsertFinishedSignal();
	InsertInfiniteLoop();

	return codeRunning;
}

void RemoteCode::WaitForFinish(RemoteU32& codeRunning)
{
	// Sleep until the code is finished
	while((uint32_t)codeRunning)
	{
		DWORD status;
		if(!GetExitCodeProcess(*procHandle, &status) || status != STILL_ACTIVE)
		  break;

		Sleep(1);
	}

	remoteThread->Suspend(); // Suspend execution (we're just infinite looping right now)
}