/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		FuncHooker.cpp
 *  \author		Andrew Shurney
 *  \brief		Provides a C99 interface to function hooking. I highly recomend the
 *              C++ implementation for type safety, but sometimes that isn't an 
 *              option.
 */


#include "FuncHooker.h"
#include "privateInc/FuncHookerCPP.h"
#include "SymbolFinder.h"
#include "SymbolFinderManager.h"

extern "C"
{
	FuncHooker* CreateFuncHooker(void *FunctionPtr, void *InjectionPtr)
	{
		if(!FunctionPtr || !InjectionPtr)
			return NULL;

		try
		{
			return new FuncHooker(FunctionPtr, InjectionPtr);
		}
		catch(...)
		{
			return NULL;
		}
	}
	
	FuncHooker* CreateFuncHookerFromName(const char *funcName, void *InjectionPtr, const char *moduleHint)
	{
		const SymbolFinder *funcNameFinder = SymbolFinderManager::Get();

		const void *FunctionPtr = funcNameFinder->GetSymbolAddr(funcName, moduleHint);
		return CreateFuncHooker(const_cast<void*>(FunctionPtr), InjectionPtr);
	}

	const void* GetTrampoline(const FuncHooker *hooker)
	{
		if(!hooker)
			return NULL;

		return hooker->GetTrampoline();
	}

	bool InstallHook(FuncHooker *hooker)
	{
		if(!hooker)
			return false;

		return hooker->InstallHook();
	}

	void RemoveHook(FuncHooker *hooker)
	{
		if(!hooker)
			return;

		return hooker->RemoveHook();
	}

	void DestroyFuncHooker(FuncHooker *hooker)
	{
		delete hooker;
	}
}
