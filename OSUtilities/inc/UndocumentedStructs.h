/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		UndocumentedStructs.h
 *  \author		Andrew Shurney
 *  \brief		Definitions for some undocumented windows structures.
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef UNDOCUMENTED_STRUCTS_H
#define UNDOCUMENTED_STRUCTS_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winternl.h>

struct CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
};

struct THREAD_BASIC_INFORMATION
{
	HRESULT  ExitStatus;
	PVOID    TebBaseAddress;
	CLIENT_ID ClientId;
	unsigned AffinityMask;
	unsigned Priority;
	unsigned BasePriority;
};

struct WIN_PEB_LDR_DATA 
{
  ULONG      Length;
  BOOLEAN    Initialized;
  PVOID      SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
};

struct WIN_LDR_MODULE 
{
  LIST_ENTRY     InLoadOrderModuleList;
  LIST_ENTRY     InMemoryOrderModuleList;
  LIST_ENTRY     InInitializationOrderModuleList;
  PVOID          BaseAddress;
  PVOID          EntryPoint;
  ULONG          SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG          Flags;
  SHORT          LoadCount;
  SHORT          TlsIndex;
  LIST_ENTRY     HashTableEntry;
  ULONG          TimeDateStamp;
};

#endif
