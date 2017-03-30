/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		FuncHooker.h
 *  \author		Andrew Shurney
 *  \brief		Provides a C99 interface to function hooking. I highly recomend the
 *              C++ implementation for type safety, but sometimes that isn't an 
 *              option.
 */

#ifndef FUNC_HOOKER_H
#define FUNC_HOOKER_H

#if 0
# if defined(WIN32) || defined(WIN64)
#  ifdef BUILDING_DLL
#   define FUNCHOOKER_DLLAPI __declspec(dllexport)
#  else
#   define FUNCHOOKER_DLLAPI __declspec(dllimport)
#  endif
#  define FUNCHOOKER_DLLCALL __cdecl
# else
#  define FUNCHOOKER_DLLAPI extern
#  define FUNCHOOKER_DLLCALL 
# endif
#else
# define FUNCHOOKER_DLLAPI
# define FUNCHOOKER_DLLCALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct FuncHooker;

/*! \brief Creates a FuncHooker object to hook the FunctionPtr passed.
    \param[in] FunctionPtr  - A pointer to the function which you are hooking
	\param[in] InjectionPtr - A pointer to the function which will hook FunctionPtr

	Note: This only creates the function hooking object. InstallHook must be called for the 
	actual hook to take place.

	\return A pointer to a function hooker object. Null on failure.
*/
FUNCHOOKER_DLLAPI FuncHooker* FUNCHOOKER_DLLCALL CreateFuncHooker(void *FunctionPtr, void *InjectionPtr);

/*! \brief Creates a FuncHooker object to hook the function with the passed name.
    \param[in] funcName     - The name of the function you are trying to hook
	\param[in] InjectionPtr - A pointer to the function which will be hooking the function named funcName
	\param[in] moduleHint   - The name of the module at which to start the search. Default null

	<p>The address of the function is first looked up amongst all the modules loaded by the program and
	then by the symbols file. Since a single name can have multiple addresses in multiple modules,
	moduleHint is used to specify which module you actually intended.</p>

	<p>Module exports are searched first for performance reasons. However, this also means if the function
	you are trying to hook has the same name as a function in any of your program's modules' exports (of
	which there are likely about 20,000, with about 15,000 of those being unique), then the wrong function
	will be hooked. There is currently no work around for this design flaw, with the exception of using the
	symbols file yourself to get the function address and calling CreateFuncHooker with that.</p>
	
	<p>Note: Currently, there is a hack in SymbolFinder.cpp:GetSymbolAddr which treats a null moduleHint
	as though you wanted to start the search with kernel32.dll.</p>

	<p>Note: This only creates the function hooking object. InstallHook must be called for the 
	actual hook to take place.</p>

	\return A pointer to a function hooker object. Null on failure.
*/
FUNCHOOKER_DLLAPI FuncHooker* FUNCHOOKER_DLLCALL CreateFuncHookerFromName(const char *funcName, void *InjectionPtr, const char *moduleHint=nullptr);

/*! \brief Returns the trampoline pointer for a function hooking object.
	\param[in] hooker - A pointer to the function hooking object.

	<p>If, from within a function which is hooking another, the user wishes to call the hooked function
	without causing an infinite loop, the trampoline must be used. The trampoline pointer should
	be casted to a function pointer of the same type as the hooked function. It can then be called.</p>
	<p>The trampoline is undefined if the hook has not yet been installed at least once.</p>

	\return A pointer to the trampoline.
*/
FUNCHOOKER_DLLAPI const void* FUNCHOOKER_DLLCALL GetTrampoline(const FuncHooker *hooker);

/*! \brief Installs a function hook.
	\param[in] hooker - A pointer to the function hooking object.

	<p>This function actually installs a function hook, and must be called before any hooking can take place.</p>

	<p>It is safe to call this function multiple times. However, no internal counting takes place, so whether
	or not the hook is actually installed is a matter of which, InstallHook or RemoveHook, was called most
	recently.</p>

	<p>Assuming the program you are hooking was made properly (ie: no race conditions), this call is entirely 
	thread safe (all other threads are paused while the hook is being installed).</p>

	\return True if the hook was successfully installed. False otherwise.
*/
FUNCHOOKER_DLLAPI bool FUNCHOOKER_DLLCALL InstallHook(FuncHooker *hooker);

/*! \brief Uninstalls a function hook.
	\param[in] hooker - A pointer to the function hooking object.
	
	<p>Turns off redirecting of calls to your hook function.</p>

	<p>It is safe to call this function multiple times. However, no internal counting takes place, so whether
	or not the hook is actually installed is a matter of which, InstallHook or RemoveHook, was called most
	recently.</p>

	<p>Assuming the program you are hooking was made properly (ie: no race conditions), this call is entirely 
	thread safe (all other threads are paused while the hook is being installed).</p>
*/
FUNCHOOKER_DLLAPI void FUNCHOOKER_DLLCALL RemoveHook(FuncHooker *hooker);

/*! \brief Destroys a function hooking object
	\param[in] hooker - A pointer to the function hooking object.

	Destroys and frees all memory associated with the function hook. If the hook was still
	installed, it is removed. All traces of the hooking having taken place are removed.
*/
FUNCHOOKER_DLLAPI void FUNCHOOKER_DLLCALL DestroyFuncHooker(FuncHooker *hooker);

#ifdef __cplusplus
}
#endif

#undef FUNCHOOKER_DLLAPI
#undef FUNCHOOKER_DLLCALL

#endif
