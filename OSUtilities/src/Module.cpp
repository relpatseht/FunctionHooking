/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		Module.cpp
 *  \author		Andrew Shurney
 *  \brief		Manages a single module in a process
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#include "Module.h"
#include <cstdint>
#include "ProcessHandleManager.h"
#include "ProcessMemory.h"
#include "UndocumentedStructs.h"
#include <algorithm>
#include <cctype>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

Module::Module(const WIN_LDR_MODULE& module, unsigned procId) : data(new ModuleData(procId)), refs(new unsigned)
{
	ProcessMemory memory(procId);

	std::wstring wideDllName(module.BaseDllName.Length/2, '\0');
	memory.Read(module.BaseDllName.Buffer, &wideDllName[0], module.BaseDllName.Length);

	data->name = std::string(wideDllName.begin(), wideDllName.end());
	std::transform(data->name.begin(), data->name.end(), data->name.begin(), std::tolower);

	data->address = module.BaseAddress;
	data->size = module.SizeOfImage;

	*refs = 1;
}

Module::Module(const Module& rhs) : data(rhs.data), refs(rhs.refs)
{
	++(*refs);
}

Module::Module(Module&& rhs) : data(rhs.data), refs(rhs.refs)
{
	rhs.data = nullptr;
	rhs.refs = nullptr;
}

Module& Module::operator=(const Module& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

Module& Module::operator=(Module&& rhs)
{
	if(this == &rhs)
		return *this;

	data = rhs.data;
	refs = rhs.refs;
	rhs.data = nullptr;
	rhs.refs = nullptr;

	return *this;
}

Module::~Module()
{
	Destroy();
}

void Module::Destroy()
{
	if(refs && !(--(*refs)))
	{
		delete data;
		delete refs;

		data = nullptr;
		refs = nullptr;
	}
}

const std::string& Module::GetName() const
{
	return data->name;
}

const void *Module::GetAddress() const
{
	return data->address;
}

bool Module::Contains(const void *addr) const
{
	return addr >= data->address && addr < (reinterpret_cast<unsigned char*>(data->address) + data->size);
}

Module::Functions Module::GetFunctions() const
{
	Functions functions;
	ProcessMemory memory(data->procHandle.GetProcId());

	IMAGE_DOS_HEADER moduleDOSHeader = memory.Read<IMAGE_DOS_HEADER>(data->address);

	if(moduleDOSHeader.e_magic != IMAGE_DOS_SIGNATURE)
		throw std::exception();

	uint8_t *addr = static_cast<uint8_t*>(data->address);
	IMAGE_EXPORT_DIRECTORY exports;

	if(data->procHandle.IsX86())
	{
		IMAGE_NT_HEADERS32 moduleNTHeader = memory.Read<IMAGE_NT_HEADERS32>(addr + moduleDOSHeader.e_lfanew);

		if(moduleNTHeader.Signature != IMAGE_NT_SIGNATURE || moduleNTHeader.OptionalHeader.NumberOfRvaAndSizes <= 0)
			throw std::exception();

		memory.Read(addr + moduleNTHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, exports);
	}
	else
	{
		IMAGE_NT_HEADERS64 moduleNTHeader = memory.Read<IMAGE_NT_HEADERS64>(addr + moduleDOSHeader.e_lfanew);

		if(moduleNTHeader.Signature != IMAGE_NT_SIGNATURE || moduleNTHeader.OptionalHeader.NumberOfRvaAndSizes <= 0)
			throw std::exception();

		memory.Read(addr + moduleNTHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, exports);
	}
	
	if(!exports.NumberOfFunctions)
		return functions;

	if(exports.NumberOfFunctions < exports.NumberOfNames)
		throw std::exception();

	std::vector<DWORD> funcAddrs(exports.NumberOfFunctions);
	std::vector<DWORD> nameAddrs(exports.NumberOfNames);
	std::vector<WORD>  ordinals(exports.NumberOfNames);

	memory.Read(addr + exports.AddressOfFunctions,    funcAddrs.data(), sizeof(DWORD)*exports.NumberOfFunctions);
	memory.Read(addr + exports.AddressOfNames,        nameAddrs.data(), sizeof(DWORD)*exports.NumberOfNames);
	memory.Read(addr + exports.AddressOfNameOrdinals, ordinals.data(),  sizeof(WORD)*exports.NumberOfNames);

	for(unsigned i=0; i<exports.NumberOfNames; ++i)
	{
		char funcName[256];
		memory.Read(addr + nameAddrs[i], funcName, sizeof(funcName)/sizeof(funcName[0]));
		funcName[255] = '\0';

		functions.insert(std::make_pair(funcName, reinterpret_cast<void*>(funcAddrs[i])));
	}

	return functions;
}

Module::ModuleData::ModuleData(unsigned procId) : procHandle(ProcessHandleManager::Get()->GetHandle(procId)) {}
