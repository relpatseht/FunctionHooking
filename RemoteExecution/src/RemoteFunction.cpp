/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteFunction.cpp
 *  \author		Andrew Shurney
 *  \brief		Provides a nice interface to executing functions on a remote process.
 */

#include "RemoteFunction.h"
#include "SymbolFinder.h"
#include "ASMStubs.h"
#include "ProcessHandleManager.h"
#include "ProcessHandle.h"
#include "RemoteCode.h"
#include "privateInc/CodeBuffer.h"
#include "SymbolFinderManager.h"
#include <deque>
#include <algorithm>

struct RemoteFunctionData
{
	ProcessHandle procHandle;
	RemoteCode &remoteCode;
	std::string name;
	const void *addr;
	unsigned callingConvention;

	RemoteFunctionData(RemoteCode &remoteCode) : procHandle(ProcessHandleManager::Get()->GetHandle(remoteCode.GetProcId())), 
		                                         remoteCode(remoteCode), name(), addr(nullptr), 
												 callingConvention(RemoteFunction::CC_CDECL) {}

	private:
		RemoteFunctionData& operator=(const RemoteFunctionData&); // do not implement
		RemoteFunctionData(const RemoteFunctionData&);            // do not implement
};

RemoteFunction::RemoteFunction(RemoteCode &remoteCode) : data(new RemoteFunctionData(remoteCode)), refs(new unsigned)
{
	*refs = 1;
}


RemoteFunction::RemoteFunction(const RemoteFunction& rhs) : data(rhs.data), refs(rhs.refs)
{
	++(*refs);
}

RemoteFunction::RemoteFunction(RemoteFunction&& rhs) : data(rhs.data), refs(rhs.refs)
{
	rhs.data = nullptr;
	rhs.refs = nullptr;
}

RemoteFunction& RemoteFunction::operator=(const RemoteFunction& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

RemoteFunction& RemoteFunction::operator=(RemoteFunction&& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;

	rhs.data = nullptr;
	rhs.refs = nullptr;

	return *this;
}

RemoteFunction::~RemoteFunction()
{
	Destroy();
}

void RemoteFunction::Destroy()
{
	if(refs && !(--(*refs)))
	{
		delete refs;
		delete data;

		refs = nullptr;
		data = nullptr;
	}
}

bool RemoteFunction::IsValid() const
{
	return data->addr != nullptr;
}

bool RemoteFunction::Find(const std::string& name)
{
	data->name = name;

	try
	{
		const SymbolFinder *finder = SymbolFinderManager::Get(data->procHandle.GetProcId());
		const void *symAddr = finder->GetSymbolAddr(name.c_str());

		if(symAddr)
		{
			data->addr = symAddr;
			return true;
		}
	}
	catch(...){}

	return false;
}

RemoteFunction& RemoteFunction::SetAddr(const void *address)
{
	data->addr = address;

	try
	{
		const SymbolFinder *finder = SymbolFinderManager::Get(data->procHandle.GetProcId());
		std::string funcName = finder->GetSymbolName(address);

		if(funcName.size())
			data->name = funcName;
	}
	catch(...)
	{

	}

	return *this;
}

RemoteFunction& RemoteFunction::SetCallingConvention(unsigned cc)
{
	if ( data->procHandle.IsX86() )
	{
		data->callingConvention = cc;
	}

	return *this;
}

RemoteFunction& RemoteFunction::SetCC(unsigned cc)
{
	return SetCallingConvention(cc);
}


void RemoteFunction::CallV(const Arguments& args, const RemoteVariable* returnArg)
{
	if(!IsValid())
		return;

	CodeBuffer& buffer = data->remoteCode.GetCodeBuffer();

	std::deque<unsigned> argIntRegisters;
	std::deque<unsigned> argFloatRegisters;
	unsigned minStackSize;

	if(data->procHandle.IsX86())
	{
		minStackSize = 4;
		if(data->callingConvention == CC_FASTCALL)
		{
			argIntRegisters.push_back(ASM::REG::ECX);
			argIntRegisters.push_back(ASM::REG::EDX);
			argFloatRegisters.clear();
		}
	}
	else
	{
		minStackSize = 8;
		argIntRegisters.push_back(ASM::REG::RCX);
		argIntRegisters.push_back(ASM::REG::RDX);
		argIntRegisters.push_back(ASM::REG::R8);
		argIntRegisters.push_back(ASM::REG::R9);

		argFloatRegisters.push_back(ASM::REG::XMM0);
		argFloatRegisters.push_back(ASM::REG::XMM1);
		argFloatRegisters.push_back(ASM::REG::XMM2);
		argFloatRegisters.push_back(ASM::REG::XMM3);

		// On x64, we need to ensure the stack is 16 uint8_t aligned first.
		ASM::AddS8 addRSP( -16, ASM::REG::RSP );
		ASM::AndRegU32 andRSP( ~0xFu, ASM::REG::RSP );

		buffer.Write( &addRSP, sizeof( addRSP ) );
		buffer.Write( &andRSP, sizeof( andRSP ) );
	}

	// On x64, 32 bytes need to be reserved in the stack no matter what for shadowing the first for arguments (even if there are less than 4)
	// Also, stack size must be 16 uint8_t aligned.
	unsigned x64StackUsed = 0;
	if ( !data->procHandle.IsX86() )
	{
		unsigned requiredStackSize = 0;

		for ( unsigned a = 0; a < args.size(); ++a )
		{
			unsigned argStackSize = args[a]->GetSizeOf();
			unsigned argStackAlignment = argStackSize % minStackSize;

			if ( argStackAlignment != 0 )
			{
				argStackSize += minStackSize - argStackAlignment;
			}

			requiredStackSize += argStackSize;
		}

		unsigned stackAlign = requiredStackSize % 16;
		if ( stackAlign != 0 )
		{
			requiredStackSize += 16 - stackAlign;
		}

		if ( requiredStackSize < 32 )
		{
			requiredStackSize = 32;
		}

		assert( requiredStackSize <= 127 );

		ASM::AddS8 stackOffset( -static_cast< int8_t >( requiredStackSize ), ASM::REG::RSP );
		buffer.Write( &stackOffset, sizeof( stackOffset ) );

		x64StackUsed = requiredStackSize;
	}


	unsigned stackSize = 0;
	for(int a=static_cast<int>(args.size())-1; a >= 0; --a)
	{
		bool argPushed = false;
		unsigned argNum = static_cast<unsigned>(a);
		if(argNum < argIntRegisters.size() && (args[a]->IsIntegral() || args[a]->IsPointer() || args[a]->IsArray()))
		{
			argPushed = true;
			args[a]->MovToReg(buffer, argIntRegisters[a]);
		}
		else if(argNum < argFloatRegisters.size() && !args[a]->IsIntegral())
		{
			argPushed = true;
			args[a]->MovToReg(buffer, argFloatRegisters[a]);
		}

		if(!argPushed)
		{
			if ( data->procHandle.IsX86() )
				args[a]->Push( buffer );
			else
			{
				assert(stackSize <= 127);
				args[a]->StoreAtStackOffset(buffer, static_cast<int8_t>(stackSize));
			}

			stackSize += std::max(minStackSize, args[a]->GetSizeOf());
		}
		else if ( !data->procHandle.IsX86() )
		{
			stackSize += minStackSize;
		}
	}

	ASM::CallAddr funcCall(data->addr);
	buffer.Write(&funcCall, sizeof(funcCall));

	if(returnArg)
		returnArg->GetReturnVal(buffer);

	if(data->callingConvention != CC_STDCALL) // Cleanup
	{
		if ( !data->procHandle.IsX86() )
		{
			stackSize = x64StackUsed;
		}

		ASM::AddS8 addEsp(static_cast<int8_t>(stackSize), ASM::REG::ESP);
		buffer.Write(&addEsp, sizeof(addEsp));
	}

	data->remoteCode.Execute();
}

unsigned RemoteFunction::GetProcId() const
{
	return data->remoteCode.GetProcId();
}