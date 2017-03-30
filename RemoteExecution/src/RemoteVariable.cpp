/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteVariable.cpp
 *  \author		Andrew Shurney
 *  \brief		Proxy to a variable on a different process.
 */

#define NO_EXTERN_REMOTE_VARIABLE_VAL
#include "RemoteVariable.h"
#include "privateInc/CodeBuffer.h"
#include "ASMStubs.h"
#include "ProcessHandle.h"
#include "ProcessHandleManager.h"
#include "privateInc/RemoteMemoryManager.h"
#include "ProcessMemory.h"

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
#else
# error Unimplemented
#endif

struct RemoteVariableData
{
	ProcessHandle procHandle;
	RemoteMemoryManager heap;
	void *varAddr;
	unsigned size;
	ProcessMemory memory;

	RemoteVariableData(unsigned procId, unsigned size) : procHandle(ProcessHandleManager::Get()->GetHandle(procId)),
		                                                 heap(RemoteMemoryManager::Get(procId)), 
		                                                 varAddr(nullptr), size(size), memory(procId)
	{
	}

  ~RemoteVariableData()
  {
    heap.Free(varAddr);
  }
};


RemoteVariable::RemoteVariable(unsigned procId, unsigned size) : data(new RemoteVariableData(procId, size)), 
	                                                             refs(new unsigned)
{
	*refs = 1;

	data->varAddr = data->heap.Allocate(size);
}

RemoteVariable::RemoteVariable(unsigned procId, unsigned size, const void *varAddr) : data(new RemoteVariableData(procId, size)), 
	refs(new unsigned)
{
	*refs = 1;

	data->varAddr = const_cast<void*>(varAddr);
}

RemoteVariable::RemoteVariable(const RemoteVariable& rhs) : data(rhs.data), refs(rhs.refs)
{
	++(*refs);
}

RemoteVariable::RemoteVariable(RemoteVariable&& rhs)      : data(rhs.data), refs(rhs.refs)
{
	rhs.data = nullptr;
	rhs.refs = nullptr;
}

RemoteVariable& RemoteVariable::operator=(const RemoteVariable& rhs)
{
	if(this == &rhs)
		return *this;

	Destroy();
	data = rhs.data;
	refs = rhs.refs;
	++(*refs);

	return *this;
}

RemoteVariable& RemoteVariable::operator=(RemoteVariable&& rhs)
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

RemoteVariable::~RemoteVariable()
{
	Destroy();
}

unsigned RemoteVariable::GetSizeOf()  const
{
	return data->size;
}

unsigned RemoteVariable::GetProcId() const
{
	return data->procHandle.GetProcId();
}

void RemoteVariable::Destroy()
{
	if(refs && !(--(*refs)))
	{
		delete data;
		delete refs;
		data = nullptr;
		refs = nullptr;
	}
}

void RemoteVariable::NewVarAddr(unsigned size)
{
	data->heap.Free(data->varAddr);
	data->varAddr = data->heap.Allocate(size);
	data->size = size;
}

void *RemoteVariable::GetVarAddr() const
{
	return data->varAddr;
}

void RemoteVariable::Write(const void *mem, unsigned size)
{
#ifdef WIN32
	data->memory.Write(GetVarAddr(), mem, size);
#else
#endif
}

void RemoteVariable::Read(void *mem, unsigned size) const
{
#ifdef WIN32
	data->memory.Read(GetVarAddr(), mem, size);
#else
#endif
}

unsigned RemoteVariable::Push(CodeBuffer& buffer, uint32_t value) const
{
	ASM::PushU32 push(value);

	if(!buffer.Write(&push, sizeof(push)))
		return 0;

	return sizeof(push);
}

unsigned RemoteVariable::Push(CodeBuffer& buffer, uint64_t value) const
{
	if(data->procHandle.IsX86())
	{
		ASM::Pushuint64_t_X86 push(value);

		if(!buffer.Write(&push, sizeof(push)))
			return 0;

		return sizeof(push);
	}
	else
	{
		ASM::Pushuint64_t_X64 push(value);

		if(!buffer.Write(&push, sizeof(push)))
			return 0;

		return sizeof(push);
	}
}
		
unsigned RemoteVariable::Push(CodeBuffer& buffer, float value) const
{
	return Push(buffer, *reinterpret_cast<uint32_t*>(&value));
}

unsigned RemoteVariable::Push(CodeBuffer& buffer, double value) const
{
	return Push(buffer, *reinterpret_cast<uint64_t*>(&value));
}


unsigned RemoteVariable::StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, uint32_t value ) const
{
	assert( !data->procHandle.IsX86() );

	ASM::MovU32ToStackWithS8Offset movToRSP( value, offset );

	if ( !buffer.Write( &movToRSP, sizeof( movToRSP ) ) )
		return 0;

	return sizeof( movToRSP );
}

unsigned RemoteVariable::StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, uint64_t value ) const
{
	assert( !data->procHandle.IsX86() );

	ASM::MovToReg_X64 movRAX( value, ASM::REG::RAX );
	ASM::MovRegToStackWithS8Offset_X64 movToRSP( offset, ASM::REG::RAX );

	if ( !buffer.Write( &movRAX, sizeof( movRAX ) ) )
		return 0;

	if ( !buffer.Write( &movToRSP, sizeof( movToRSP ) ) )
		return sizeof( movRAX );

	return sizeof( movRAX ) + sizeof( movToRSP );
}

unsigned RemoteVariable::StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, float value ) const
{
	return StoreAtStackOffset( buffer, offset, *reinterpret_cast< uint32_t* >( &value ) );
}

unsigned RemoteVariable::StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, double value ) const
{
	return StoreAtStackOffset( buffer, offset, *reinterpret_cast< uint64_t* >( &value ) );
}

unsigned RemoteVariable::MovTo(CodeBuffer& buffer, uint32_t value, unsigned reg) const
{
	ASM::MovToReg_X86 mov(value, static_cast<uint8_t>(reg));
	if(!buffer.Write(&mov, sizeof(mov)))
		return 0;

	return sizeof(mov);
}

unsigned RemoteVariable::MovTo(CodeBuffer& buffer, uint64_t value, unsigned reg) const
{
	if(data->procHandle.IsX86())
	{
		assert(false && "X86 does not have any 64 bit registers.");
		return 0;
	}
	else
	{
		ASM::MovToReg_X64 mov(value, static_cast<uint8_t>(reg));
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}
		
unsigned RemoteVariable::MovTo(CodeBuffer& buffer, float* value, unsigned reg) const
{
	if(data->procHandle.IsX86())
	{
		ASM::FldF32Ptr_X86 fld(value);
		if(!buffer.Write(&fld, sizeof(fld)))
			return 0;

		return sizeof(fld);
	}
	else
	{
		ASM::MovF32PtrToSSEReg mov(value, buffer.GetCurAddr(), static_cast<uint8_t>(reg));
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}

unsigned RemoteVariable::MovTo(CodeBuffer& buffer, double* value, unsigned reg) const
{
	if(data->procHandle.IsX86())
	{
		ASM::FldF64Ptr_X86 fld(value);
		if(!buffer.Write(&fld, sizeof(fld)))
			return 0;

		return sizeof(fld);
	}
	else
	{
		ASM::MovF64PtrToSSEReg mov(value, buffer.GetCurAddr(), static_cast<uint8_t>(reg));
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}

unsigned RemoteVariable::GetReturnVal(CodeBuffer& buffer, uint32_t* value) const
{
	if(data->procHandle.IsX86())
	{
		ASM::MovEAXToAddr mov(value);
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
	else
	{
		ASM::MovRAXToAddr mov(value);
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}

unsigned RemoteVariable::GetReturnVal(CodeBuffer& buffer,  uint64_t* value) const
{
	if(data->procHandle.IsX86())
	{
		ASM::MovEAXToAddr mov(value);
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
	else
	{
		ASM::MovRAXToAddr mov(value);
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}
		
unsigned RemoteVariable::GetReturnVal(CodeBuffer& buffer,  float* value) const
{
	if(data->procHandle.IsX86())
	{
		ASM::FstpF32Ptr_X86 fstp(value);
		if(!buffer.Write(&fstp, sizeof(fstp)))
			return 0;

		return sizeof(fstp);
	}
	else
	{
		ASM::MovSSERegToF32Ptr mov(value, buffer.GetCurAddr());
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}

unsigned RemoteVariable::GetReturnVal(CodeBuffer& buffer,  double* value) const
{
	if(data->procHandle.IsX86())
	{		
		ASM::FstpF64Ptr_X86 fstp(value);
		if(!buffer.Write(&fstp, sizeof(fstp)))
			return 0;

		return sizeof(fstp);
	}
	else
	{
		ASM::MovSSERegToF64Ptr mov(value, buffer.GetCurAddr());
		if(!buffer.Write(&mov, sizeof(mov)))
			return 0;

		return sizeof(mov);
	}
}

template RemoteVariableImpl<intptr_t>;
template RemoteVariableImpl<uintptr_t>;
template RemoteVariableImpl<int64_t>;
template RemoteVariableImpl<uint64_t>;
template RemoteVariableImpl<int32_t>;
template RemoteVariableImpl<uint32_t>;
template RemoteVariableImpl<int16_t>;
template RemoteVariableImpl<uint16_t>;
template RemoteVariableImpl<int8_t>;
template RemoteVariableImpl<uint8_t>;
template RemoteVariableImpl<float>;
template RemoteVariableImpl<double>;

template RemoteVariableImpl<intptr_t*>;
template RemoteVariableImpl<uintptr_t*>;
template RemoteVariableImpl<int64_t*>;
template RemoteVariableImpl<uint64_t*>;
template RemoteVariableImpl<int32_t*>;
template RemoteVariableImpl<uint32_t*>;
template RemoteVariableImpl<int16_t*>;
template RemoteVariableImpl<uint16_t*>;
template RemoteVariableImpl<int8_t*>;
template RemoteVariableImpl<uint8_t*>;
template RemoteVariableImpl<float*>;
template RemoteVariableImpl<double*>;
