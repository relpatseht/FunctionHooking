/************************************************************************************\
 * OSUtilities - An Andrew Shurney Production                                       *
\************************************************************************************/

/*! \file		OSMemoryRights.cpp
 *  \author		Andrew Shurney
 *  \brief		Platform independed memory right translation
 */

#include "OSMemoryRights.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
#endif

unsigned OSMemoryRights::TranslateAccessToOS(unsigned access)
{
	unsigned osAccess = 0;
#ifdef WIN32
	switch(access)
	{
		case EXECUTE:                                osAccess = PAGE_EXECUTE;           break;
		case EXECUTE | READ:                         osAccess = PAGE_EXECUTE_READ;      break;
		case EXECUTE | READ | WRITE:                 osAccess = PAGE_EXECUTE_READWRITE; break;
		case EXECUTE | READ | WRITE | COPY_ON_WRITE: osAccess = PAGE_EXECUTE_WRITECOPY; break;
		case NO_ACCESS:                              osAccess = PAGE_NOACCESS;          break;
		case READ:                                   osAccess = PAGE_READONLY;          break;
		case READ | WRITE:                           osAccess = PAGE_READWRITE;         break;
		case READ | WRITE | COPY_ON_WRITE:           osAccess = PAGE_WRITECOPY;         break;
		default: osAccess = ~0u;
	}
#else
	if(access & NO_ACCESS    ) osAccess |= PROT_NONE;
	if(access & READ         ) osAccess |= PROT_READ;
	if(access & WRITE        ) osAccess |= PROT_WRITE;
	if(access & EXECUTE      ) osAccess |= PROT_EXEC;
	if(access & COPY_ON_WRITE) osAccess  = ~0u; // not controllable on linux?
#endif

	return osAccess;
}

unsigned OSMemoryRights::TranslateAccessFromOS(unsigned osAccess)
{
	unsigned access = 0;
#ifdef WIN32
	switch(osAccess)
	{
		case PAGE_EXECUTE:           access = EXECUTE;                                break;
		case PAGE_EXECUTE_READ:      access = EXECUTE | READ;                         break;
		case PAGE_EXECUTE_READWRITE: access = EXECUTE | READ | WRITE;                 break;
		case PAGE_EXECUTE_WRITECOPY: access = EXECUTE | READ | WRITE | COPY_ON_WRITE; break;
		case PAGE_NOACCESS:          access = NO_ACCESS;                              break;
		case PAGE_READONLY:          access = READ;                                   break;
		case PAGE_READWRITE:         access = READ | WRITE;                           break;
		case PAGE_WRITECOPY:         access = READ | WRITE | COPY_ON_WRITE;           break;
		default:                     access =  ~0u;
	}
#else
	if(osAccess & PROT_NONE ) access |= NO_ACCESS;
	if(osAccess & PROT_READ ) access |= READ;
	if(osAccess & PROT_WRITE) access |= WRITE;
	if(osAccess & PROT_EXEC ) access |= EXECUTE;
#endif

	return access;
}