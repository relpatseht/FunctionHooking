/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		FuncHookerFactory.h
 *  \author		Andrew Shurney
 *  \brief		Provides a C++11 interface to function hooking.
 */

#ifndef FUNC_HOOKER_FACTORY_H
#define FUNC_HOOKER_FACTORY_H

#include "FuncHooker.h"
#include "ArgTypeTraits.h"
#include "MMP.h"

#define MAX_ARGS 16

/*! \brief Wraps the C function hooking interface.

    You should never create one of these directly. Create them using factory functions.

	\sa CreateFuncHooker
*/
class FuncHookerWrapper
{
	private:
		FuncHooker *hooker;

	public:
		FuncHookerWrapper(void *FunctionPtr, void *InjectionPtr) : 
		                  hooker(::CreateFuncHooker(FunctionPtr, InjectionPtr)) {}

		FuncHookerWrapper(const char *funcName, void *InjectionPtr, const char *moduleHint=nullptr) :
						  hooker(::CreateFuncHookerFromName(funcName, InjectionPtr, moduleHint)){}
		
		virtual ~FuncHookerWrapper()
		{
			::DestroyFuncHooker(hooker);
		}

		/*! \brief Simply calls the C interface for getting a trampoline.

		    You should never call this. You should have a FuncHookerImpl object
			on which you can use the CallInjectee method which handles all the casting
			for you.
		*/
		const void* GetTrampoline() const
		{
			return ::GetTrampoline(hooker);
		}

		/*! \brief Installs the hook.
		*/
		bool InstallHook()
		{
			return ::InstallHook(hooker);
		}
		/*! \brief Removes the hook.
		*/
		void RemoveHook()
		{
			return ::RemoveHook(hooker);
		}
};

template<typename F>
class FuncHookerImpl;

template<typename F>
struct FuncHookerFactory;

template<typename F>
FuncHookerImpl<F>* CreateFuncHooker(F InjecteeFunc, F InjectorFunc);

template<typename F>
FuncHookerImpl<F>* CreateFuncHooker(const char *injecteeFuncName, F InjectorFunc, const char *moduleHint=nullptr);

template<typename F>
FuncHookerImpl<F>* CreateFuncHooker(void *InjecteeFunc, F InjectorFunc)
{
	return CreateFuncHooker(reinterpret_cast<F>(InjecteeFunc), InjectorFunc);
}

/* \brief Creates a template declaration defining types needed for a function pointer.

   <p>Defines all types needed for a function pointer in a template declaration. The function
   pointer can be regular or a member function pointer. The class type, if applicable, will be
   C. The return type, if non-void, will be R. The arguments will be A1-AN.</p>
   <p>This template declaration assumes it is to be used with a specialization of a templated
   function. Even if there are no arguments, a void return type, and it is not a member function,
   template<> will still be written by this macro.</p>
*/
#define FP_TEMPLATE_DECLARATION(MEMBER, VOID, N)                                                                  \
	template<MMP_IF(MEMBER, typename C) MMP_COMMA_IF(MMP_AND(MEMBER, MMP_OR(MMP_NOT(VOID), MMP_BOOL(N))))         \
	    MMP_IF_NOT(VOID, typename R) MMP_COMMA_IF(MMP_AND(MMP_NOT(VOID), MMP_BOOL(N))) MMP_ENUM_C(N, typename A)>

/* \brief Builds a type for a function pointer.

   <p>Creates the full listing of a function pointer given all the arguments, calling convention, etc
   for the function. The type will not be given a name.</p>
   <p>This macro assumes it is being used with arguments of type A1-AN, a return type R (or void), and a class
   of type C (if it is a member function pointer).</p>
*/
#define FUNCTION_TYPE(N, CC, CONST, VOLATILE, MEMBER, VOID)                  \
	MMP_IF_ELSE(VOID, void, R) (CC MMP_IF(MEMBER, C::) *) (MMP_ENUM_C(N, A)) \
	MMP_IF(CONST, const) MMP_IF(VOLATILE, volatile)

/*! \brief Builds a typedef for a function pointer

    The typedef's name will be FunctionType
*/
#define FUNCTION_TYPEDEF(N, CC, CONST, VOLATILE, MEMBER, VOID) 				  \
	typedef MMP_IF_ELSE(VOID, void, R) (CC MMP_IF(MEMBER, C::) *FunctionType) \
	 (MMP_ENUM_C(N, A)) MMP_IF(CONST, const) MMP_IF(VOLATILE, volatile);

#define PARAMETER_LIST(N) typename FastIntermediary<MMP_CONCAT(A, N)>::type MMP_CONCAT(a, N)

/*! \class FuncHookerImpl
    \brief A C++11 typesafe function hooking object.

	<p>You shouldn't create these directly, you should use the function factory methods.</p>
	<p>This interface provides InstallHook and RemoveHook method, as well as the new all
	important CallInjectee method. CallInjectee is entirely typesafe and makes grabbing and
	calling the trampoline as simple and safe as any function. It will even try its best to
	avoid passing objects by value, despite the internal functions types.</p>
*/

#define FUNC_HOOKER_IMPL(N, CC, CONST, VOLATILE, MEMBER, VOID)                                                  \
FP_TEMPLATE_DECLARATION(MEMBER, VOID, N)                                                                        \
class FuncHookerImpl<FUNCTION_TYPE(N, CC, CONST, VOLATILE, MEMBER, VOID)> : public FuncHookerWrapper            \
{                                                                                                               \
	public:                                                                                                     \
		FUNCTION_TYPEDEF(N, CC, CONST, VOLATILE, MEMBER, VOID)                                                  \
		                                                                                                        \
	private:                                                                                                    \
		template<typename T> friend struct FuncHookerFactory;                                                   \
		                                                                                                        \
	protected:                                                                                                  \
		FuncHookerImpl(const FunctionType& InjecteeFunc, const FunctionType& InjectorFunc) :                    \
		                  FuncHookerWrapper(reinterpret_cast<void*>(InjecteeFunc),                              \
								            reinterpret_cast<void*>(InjectorFunc)){}                            \
 		                                                                                                        \
		FuncHookerImpl(const char *injecteeFuncName, const FunctionType& InjectorFunc, const char *module) :    \
		                  FuncHookerWrapper(injecteeFuncName, reinterpret_cast<void*>(InjectorFunc), module){}  \
						                                                                                        \
	public:                                                                                                     \
		virtual ~FuncHookerImpl() {}                                                                            \
		                                                                                                        \
		MMP_IF_ELSE(VOID, void, R) CallInjectee(MMP_IF(MEMBER, C* c) MMP_COMMA_IF(MMP_AND(MEMBER, MMP_BOOL(N))) \
		    MMP_ENUM_MC(N, PARAMETER_LIST)) const                                                               \
		{                                                                                                       \
			FunctionType InjecteeFunc = reinterpret_cast<FunctionType>(GetTrampoline());                        \
			MMP_IF_NOT(VOID, return) MMP_IF(MEMBER, c->) InjecteeFunc(MMP_ENUM_C(N, a));                        \
		}                                                                                                       \
};


#define FUNC_HOOKER_CREATORS(N, CC, CONST, VOLATILE, MEMBER, VOID)                                                            \
FP_TEMPLATE_DECLARATION(MEMBER, VOID, N)                                      						                          \
struct FuncHookerFactory<FUNCTION_TYPE(N, CC, CONST, VOLATILE, MEMBER, VOID)> 						                          \
{																									                          \
	FUNCTION_TYPEDEF(N, CC, CONST, VOLATILE, MEMBER, VOID)											                          \
																									                          \
	static FuncHookerImpl<FunctionType>* Create(FunctionType InjecteeFunc, FunctionType InjectorFunc)                         \
	{																								                          \
		return new FuncHookerImpl<FunctionType>(InjecteeFunc, InjectorFunc);						                          \
	}																								                          \
	static FuncHookerImpl<FunctionType>* Create(const char *injecteeName, FunctionType InjectorFunc, const char *moduleHint)  \
	{																								                          \
		return new FuncHookerImpl<FunctionType>(injecteeName, InjectorFunc, moduleHint);			                          \
	}																								                          \
};

MMP_ALL_FUNCTIONAL_VARIANTS(MAX_ARGS, FUNC_HOOKER_IMPL)
MMP_ALL_FUNCTIONAL_VARIANTS(MAX_ARGS, FUNC_HOOKER_CREATORS)

#undef PARAMETER_LIST
#undef FUNC_HOOKER_CREATORS
#undef FUNCTION_TYPEDEF
#undef FUNCTION_TYPE
#undef FP_TEMPLATE_DECLARATION

/*! \brief Creates a function hooking objects provied function pointers.
    \param[in] InjecteeFunc - The function being hooked
	\param[in] InjectorFunc - The function doing the hooking.

	This factory function is entirely typesafe. If it is throwing errors, your
	function signature likely do not match exactly. Remember, calling convention
	matters in a function signature.

	\return C++11 function hooking object.
*/
template<typename F>
FuncHookerImpl<F>* CreateFuncHooker(F InjecteeFunc, F InjectorFunc)
{
	try
	{
		return FuncHookerFactory<F>::Create(InjecteeFunc, InjectorFunc);
	}
	catch(...)
	{
		return nullptr;
	}
}

/*! \brief Creates a FuncHooker object to hook the function with the passed name.
    \param[in] funcName     - The name of the function you are trying to hook
	\param[in] InjectorFunc - The function doing the hooking.
	\param[in] moduleHint   - Which module would you like to start you search at (default null).

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

	\return C++11 function hooking object.
*/
template<typename F>
FuncHookerImpl<F>* CreateFuncHooker(const char *injecteeName, F InjectorFunc, const char *moduleHint)
{
	try
	{
		return FuncHookerFactory<F>::Create(injecteeName, InjectorFunc, moduleHint);
	}
	catch(...)
	{
		return nullptr;
	}
}

/*! \brief Casts an address to a functon hooking object to the appropriate type.
    \param[in] addr - Address of the C++ function hooking object.
	\param[in] - Pointer to the hooked or hooking function.

	This function is useful if you are storing your function hooking objects as void pointers.

	\return C++ function hooking object of the appropriate type.
*/
template<typename F> 
FuncHookerImpl<F>* FuncHookerCast(void *addr, F)
{
	return reinterpret_cast<FuncHookerImpl<F>*>(addr);
}

#undef MAX_ARGS

#endif
