#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdint>
#include "RemoteFunction.h"
#include "RemoteCode.h"

int main(int argc, char *argv[])
{
	if(argc == 0)
		return -2;

	std::string path(argv[0]);
	size_t lastSlash = path.rfind('\\');
	if(lastSlash == std::string::npos)
		lastSlash = path.rfind('/');

	path = path.substr(0, lastSlash);
	path += "\\TestExe.exe ";

	STARTUPINFO testExeStartInfo;
	SecureZeroMemory(&testExeStartInfo, sizeof(STARTUPINFO));
	testExeStartInfo.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION testExeInfo;
	if(!CreateProcess(path.c_str(), NULL, NULL, NULL, false, 
		              CREATE_NEW_CONSOLE | CREATE_SUSPENDED, 
					  NULL, NULL, &testExeStartInfo, &testExeInfo))
		return -1;

	RemoteCode remoteCode(testExeInfo.dwProcessId);
	if(!remoteCode.Initialize())
		return -1;

	RemoteFunction installHooksHandle = remoteCode.GetFunction("TestExeHookDll.dll", "InstallHooks");
	if(!installHooksHandle.IsValid())
		return -1;

	uint32_t installSuccessful = installHooksHandle.Call<uint32_t>();
	if(!installSuccessful)
		return -1;

	ResumeThread(testExeInfo.hThread);

	CloseHandle(testExeInfo.hThread);
	CloseHandle(testExeInfo.hProcess);

	return 0;
}