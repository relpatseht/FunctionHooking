/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteFunction.h
 *  \author		Andrew Shurney
 *  \brief		Provides a nice interface to executing functions on a remote process.
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef REMOTE_FUNCTION_H
#define REMOTE_FUNCTION_H

#include <vector>
#include <string>
#include <type_traits>
#include "RemoteVariable.h"
#include "MMP.h"

class RemoteCode;
struct RemoteFunctionData;

/*! \brief Provides a convenient interface to calling remote functions.

    <p>Unfortunately, it is impossible to get any real information other than the
	function address from the remote process. The calling convention is assumed to
	be cdecl, but if not, you will have to set it. The number and type of parameters
	as well as return value is unknown. Whatever you pass is assumed to be correct.
	This class is not safe because I don't know how I would do that.</p>
	<p>Because there is not standard defining how non-pod types are passed, this
	proxy will only work with pointers and all standard types. Arrays don't generally
	work with the exception of strings.</p>

	<p>Normally, you will not create on of these objects directly. The RemoteCode
	object will do it for you.</p>
	<p>This object is reference counted internally.</p>
*/
class RemoteFunction
{
	private:
		template<typename T>
		struct IsRemoteVariable
		{
			static const bool value = false;
		};

		template<typename T>
		struct IsRemoteVariable<RemoteVariableImpl<T> >
		{
			static const bool value = true;
		};

		template<typename T>
		struct IsString
		{
			static const bool value = false;
		};

		template<>
		struct IsString<char*>
		{
			static const bool value = true;
		};

		template<>
		struct IsString<const char*>
		{
			static const bool value = true;
		};

		RemoteFunctionData *data;
		unsigned *refs;

		void Destroy();
		unsigned GetProcId() const;

	public:
		/// An ordered collection of remote variables to pass to CallV
		typedef std::vector<const RemoteVariable*> Arguments;
		enum
		{
			CC_STDCALL, //!< The stdcall calling convention (all windows api functions)
			CC_CDECL,   //!< The cdecl calling convention (the default)
			CC_FASTCALL //!< The fastcall calling convention (rarely used)
		};

		RemoteFunction(RemoteCode &remoteCode);
		RemoteFunction(const RemoteFunction& rhs);
		RemoteFunction(RemoteFunction&& rhs);
		RemoteFunction& operator=(const RemoteFunction& rhs);
		RemoteFunction& operator=(RemoteFunction&& rhs);
		~RemoteFunction();

		/*! \brief Tells if this handle is valid and ready to use.
		    
			In reality this only tells if the handle has a function address.
			\return True if handle can be used, false otherwise
		*/
		bool IsValid() const;

		/*! \brief Tell this handle to point to a function with the given name in the remote process
		    \param[in] Name - name of the function in the remote process.

			Searches the modules of and the symbols database of the remote process for a function
			with the provided name and sets this handles address to it.

			\return True is successful.
		*/
		bool Find(const std::string& name);

		/*! \brief Sets the address of this function to the one provided.
		    \param[in] address - New function address for this handle.

			Gives this function an address. No checks are done to make sure the
			address is valid.

			\return Reference to this RemoteFunction object for chaining purposes.
		*/
		RemoteFunction& SetAddr(const void *address);

		/*! \brief Sets the calling convention of this function handle.
		    \param[in] cc - Calling convention as defined in the above enum

			\return Reference to this RemoteFunctionObject for chaining.
		*/

		RemoteFunction& SetCallingConvention(unsigned cc);

		/*! \brief Alias for SetCallingConvention.
		    \sa SetCallingConvention
		*/
		RemoteFunction& SetCC(unsigned cc);

		/*! \brief Call this function with a list of parameters
		    \param[in] args         - An ordered vector of remote variable objects to pass to the function
			\param[inout] returnArg - A pointer to the remote variable in which to store the return value. Default null.

			Normally, you will just want to use the nice templated Call method, but
			this is one also an option. It will, provided the variables and the calling
			convention of this function, construct the code to call the function and
			execute it.
		*/
		void CallV(const Arguments& args, const RemoteVariable* returnArg = nullptr);

		/*! \method Call
		    \brief Calls this function with a bunch of behind the scenes conversions to make it easy and clean.

			<p>Call requires one templated parameter specified, the return type, and the rest
			is gathered from the argument list provided. The arguments can be RemoteVarialbes,
			immediates, or variables. Pointers will not work here, they need to be wrapped in
			RemoteVariables separately (I think...). All values will be converted to remote
			variables automatically and the return type will be a pod type.</p>
			<p>Unfortunately, since there is no way of knowing how many or what kind of arguments
			this function is supposed to take, you can pass _anything_. Nothing is safe here,
			kid, we're not messing around.</p>
			<p>Internally calls CallV.</p>
		*/

#define MAX_ARGS 16 //!< Maximum number of arguments Call should support.

#define TEMPLATE_DECLARATION(VOID, N) 								 \
		MMP_IF(MMP_OR(MMP_NOT(VOID), MMP_BOOL(N)), 					 \
			   template<MMP_IF_NOT(VOID, typename R MMP_COMMA_IF(N)) \
			            MMP_ENUM_C(N, typename A)>)

#define PARAMETER_LIST(N) const MMP_CONCAT(A, N) & MMP_CONCAT(a, N)

#define ARGUMENT_DECLARER(N) 																                       \
		RemoteVariable* MMP_CONCAT(pA, N) = 							                                           \
			IsRemoteVariable<MMP_CONCAT(A,N)>::value ? 										                       \
				reinterpret_cast<RemoteVariable*>(const_cast<MMP_CONCAT(A,N)*>(std::addressof(MMP_CONCAT(a,N)))) : \
				(IsString<MMP_CONCAT(A,N)>::value ? 										                       \
					(RemoteVariable*)new RemoteStr(GetProcId(), reinterpret_cast<const char*>(MMP_CONCAT(a,N))) :  \
					(RemoteVariable*)new RemoteVariableImpl<MMP_CONCAT(A, N)>(GetProcId(), MMP_CONCAT(a, N)));     \
		args.push_back(MMP_CONCAT(pA, N));

#define ARGUMENT_DESTROYER(N)						  \
		if(!IsRemoteVariable<MMP_CONCAT(A,N)>::value) \
			delete MMP_CONCAT(pA,N);

#define CALL_IMPLEMENTATION(N, VOID)                            \
TEMPLATE_DECLARATION(VOID, N)                                   \
MMP_IF_ELSE(VOID, void, R) Call(MMP_ENUM_MC(N, PARAMETER_LIST)) \
{	                                                            \
	Arguments args;                                             \
	args.reserve(N);                                            \
	                                                            \
	MMP_ENUM_M(N, ARGUMENT_DECLARER, )                          \
	MMP_IF_NOT(VOID, RemoteVariableImpl<R> r(GetProcId());)     \
	                                                            \
	CallV(args MMP_IF_NOT(VOID, MMP_COMMA std::addressof(r)));  \
	                                                            \
	MMP_ENUM_M(N, ARGUMENT_DESTROYER, )                         \
	                                                            \
	MMP_IF_NOT(VOID, return (R)r;)                              \
}

		template<typename A>
		void PackArgument(Arguments *remoteArgs, const A& a)
		{
			RemoteVariable 
		}
		template<typename A, typename ...Args>
		void PackArguments(const A& a, const Args&... args, Arguments *remoteArgs)
		{
			
		}

		template<typename R, typename ...Args>
		R Call(const Args&... args)
		{
			Arguments remoteArgs;


		}


		MMP_REPEAT_Z(MAX_ARGS, CALL_IMPLEMENTATION, 0)
		MMP_REPEAT_Z(MAX_ARGS, CALL_IMPLEMENTATION, 1)

#undef CALL_IMPLEMENTATION
#undef ARGUMENT_DESTROYER
#undef ARGUMENT_DECLARER
#undef PARAMETER_LIST
#undef TEMPLATE_DECLARATION
#undef MAX_ARGS

};

#endif
