/************************************************************************************\
 * RemoteExecution - An Andrew Shurney Production                                   *
\************************************************************************************/

/*! \file		RemoteVariable.h
 *  \author		Andrew Shurney
 *  \brief		Proxy to a variable on a different process.
 */

#ifndef REMOTE_VARIABLE_H
#define REMOTE_VARIABLE_H

#include <cstdint>
#include <cassert>
#include <string>
#include <type_traits>

#ifdef _MSC_VER
#pragma warning (disable : 4127)
#endif

class CodeBuffer;
class ProcessHandle;
struct RemoteVariableData;

class RemoteVariable
{
	private:
		RemoteVariableData *data;
		unsigned *refs;

	protected:
		unsigned GetProcId() const;

		void Destroy();
		void NewVarAddr(unsigned size);
		void *GetVarAddr() const;
		void Write(const void *mem, unsigned size);
		void Read(void *mem, unsigned size) const;

		unsigned Push(CodeBuffer& buffer, uint32_t value) const;
		unsigned Push(CodeBuffer& buffer, uint64_t value) const;
		
		unsigned Push(CodeBuffer& buffer, float value) const;
		unsigned Push(CodeBuffer& buffer, double value) const;

		unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, uint32_t value ) const;
		unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, uint64_t value ) const;

		unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, float value ) const;
		unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset, double value ) const;

		unsigned MovTo(CodeBuffer& buffer, uint32_t value, unsigned reg) const;
		unsigned MovTo(CodeBuffer& buffer, uint64_t value, unsigned reg) const;
		
		unsigned MovTo(CodeBuffer& buffer, float* value, unsigned reg) const;
		unsigned MovTo(CodeBuffer& buffer, double* value, unsigned reg) const;

		unsigned GetReturnVal(CodeBuffer& buffer, uint32_t* value) const;
		unsigned GetReturnVal(CodeBuffer& buffer, uint64_t* value) const;
		
		unsigned GetReturnVal(CodeBuffer& buffer, float* value) const;
		unsigned GetReturnVal(CodeBuffer& buffer, double* value) const;

	public:
		RemoteVariable(unsigned procId, unsigned size);
		RemoteVariable(unsigned procId, unsigned size, const void *varAddr);
		RemoteVariable(const RemoteVariable& rhs);
		RemoteVariable(RemoteVariable&& rhs);
		RemoteVariable& operator=(const RemoteVariable& rhs);
		RemoteVariable& operator=(RemoteVariable&& rhs);
		virtual ~RemoteVariable();

		virtual bool     IsPointer()       const = 0;
		virtual bool     IsArray()         const = 0;
		virtual bool     IsIntegral()      const = 0;
		virtual bool     IsFloatingPoint() const = 0;
		virtual unsigned GetSizeOf()       const;

		virtual unsigned Push(CodeBuffer& buffer) const = 0;
		virtual unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset ) const = 0;
		virtual unsigned MovToReg(CodeBuffer& buffer, unsigned reg) const = 0;
		virtual unsigned GetReturnVal(CodeBuffer& buffer) const = 0;
};

template<typename T>
class RemoteVariableImpl : public RemoteVariable
{
	private:
		typedef typename std::remove_pointer<T>::type PointedType;
		static const bool Derefernceable = !std::is_same<PointedType, T>::value; // If the pointed type and T are the same, then T is not a pointer.

	public:
		typedef T ValueType;
		typedef RemoteVariableImpl<T*> PointerType;
		typedef typename std::conditional<Derefernceable, RemoteVariableImpl<PointedType>, void>::type DerefType;

	protected:
		T GetValue() const
		{
			ValueType value;
			Read(&value, sizeof(ValueType));

			return value;
		}

		template<typename U>
		unsigned PushHelper(CodeBuffer& buffer, const U& value) const
		{
			if(sizeof(U) > 4)
				return RemoteVariable::Push(buffer, (uint64_t)value);
			else
				return RemoteVariable::Push(buffer, (uint32_t)value);
		}

		unsigned PushHelper(CodeBuffer& buffer, float value) const
		{
			return RemoteVariable::Push(buffer, value);
		}

		unsigned PushHelper(CodeBuffer& buffer, const double& value) const
		{
			return RemoteVariable::Push(buffer, value);
		}

		template<typename U>
		unsigned StoreAtStackOffsetHelper( CodeBuffer& buffer, int8_t offset, const U& value ) const
		{
			if ( sizeof( U ) > 4 )
				return RemoteVariable::StoreAtStackOffset( buffer, offset, ( uint64_t ) value );
			else
				return RemoteVariable::StoreAtStackOffset( buffer, offset, ( uint32_t ) value );
		}

		unsigned StoreAtStackOffsetHelper( CodeBuffer& buffer, int8_t offset, float value ) const
		{
			return RemoteVariable::StoreAtStackOffset( buffer, offset, value );
		}

		unsigned StoreAtStackOffsetHelper( CodeBuffer& buffer, int8_t offset, const double& value ) const
		{
			return RemoteVariable::StoreAtStackOffset( buffer, offset, value );
		}


		template<typename U, typename V>
		struct DerefHelper
		{
			U Deref(unsigned procId, const V& val)
			{
				return U(procId, val);
			}
		};

		template<typename V>
		struct DerefHelper<void, V>
		{
			void Deref(unsigned, const V&)
			{
				return;
			}
		};

		RemoteVariableImpl(unsigned procId, const T& /*value*/, unsigned size) : RemoteVariable(procId, size) 
		{
		}

	public:
		RemoteVariableImpl(unsigned procId, const T* addr) : RemoteVariable(procId, sizeof(T), addr) 
		{
		}

		RemoteVariableImpl(unsigned procId, const T& value = T()) : RemoteVariable(procId, sizeof(T)) 
		{
			*this = value;
		}

		virtual ~RemoteVariableImpl(){}

		virtual bool IsPointer()       const { return std::is_pointer<T>::value;        }
		virtual bool IsArray()         const { return std::is_array<T>::value;          }
		virtual bool IsIntegral()      const { return std::is_integral<T>::value;       }
		virtual bool IsFloatingPoint() const { return std::is_floating_point<T>::value; }
		
		virtual unsigned Push(CodeBuffer& buffer) const
		{
			if(IsArray() || IsPointer())
				return PushHelper(buffer, reinterpret_cast<uintptr_t>(GetVarAddr()));
			else
				return PushHelper(buffer, GetValue());
		}

		virtual unsigned StoreAtStackOffset( CodeBuffer &buffer, int8_t offset ) const
		{
			if ( IsArray() || IsPointer() )
				return StoreAtStackOffsetHelper( buffer, offset, reinterpret_cast<uintptr_t>( GetVarAddr() ) );
			else
				return StoreAtStackOffsetHelper( buffer, offset, GetValue() );
		}

		virtual unsigned MovToReg(CodeBuffer& buffer, unsigned reg) const
		{
			if(sizeof(T) > 4)
			{
				if ( IsFloatingPoint() )
					return RemoteVariable::MovTo( buffer, reinterpret_cast< double* >( GetVarAddr() ), reg );
				else if ( IsPointer() )
					return RemoteVariable::MovTo( buffer, reinterpret_cast< uintptr_t >( GetVarAddr() ), reg );
				else
					return RemoteVariable::MovTo(buffer, (uint64_t)GetValue(), reg);
			}
			else
			{
				if( IsFloatingPoint())
					return RemoteVariable::MovTo(buffer, reinterpret_cast<float*>(GetVarAddr()), reg);
				else if ( IsPointer() )
					return RemoteVariable::MovTo( buffer, reinterpret_cast< uintptr_t >( GetVarAddr() ), reg );
				else
					return RemoteVariable::MovTo(buffer, (uint32_t)GetValue(), reg);
			}
		}

		virtual unsigned GetReturnVal(CodeBuffer& buffer) const
		{
			if(sizeof(T) > 4)
			{
				if( IsFloatingPoint() )
					return RemoteVariable::GetReturnVal(buffer, reinterpret_cast<double*>(GetVarAddr()));
				else
					return RemoteVariable::GetReturnVal(buffer, reinterpret_cast<uint64_t*>(GetVarAddr()));
			}
			else
			{
				if( IsFloatingPoint() )
					return RemoteVariable::GetReturnVal(buffer, reinterpret_cast<float*>(GetVarAddr()));
				else
					return RemoteVariable::GetReturnVal(buffer, reinterpret_cast<uint32_t*>(GetVarAddr()));
			}
		}


		virtual unsigned GetSizeOf() const
		{
			return sizeof( T );
		}

		PointerType operator&() const
		{
			return PointerType(GetProcId(), reinterpret_cast<T*>(GetVarAddr()));
		}

		typename DerefType operator*() const
		{
			DerefHelper<DerefType, T> helper;
			return helper.Deref(GetProcId(), GetValue());
		}

		virtual operator T() const
		{
			return GetValue();
		}

		virtual RemoteVariableImpl& operator=(const T& val)
		{
			Write(&val, sizeof(T));

			return *this;
		}

		template<typename U>
		RemoteVariableImpl<decltype(T+U)> operator+(const U& val) const
		{
			return RemoteVariableImpl<decltype(T+U)>(GetProcId(), GetValue()+val);
		}

		template<typename U>
		RemoteVariableImpl<decltype(T-U)> operator-(const U& val) const
		{
			return RemoteVariableImpl<decltype(T-U)>(GetProcId(), GetValue()-val);
		}

		template<typename U>
		RemoteVariableImpl& operator+=(const U& val) const
		{
			return (*this = (GetValue()+val));
		}

		template<typename U>
		RemoteVariableImpl& operator-=(const U& val) const
		{
			return (*this = (GetValue()-val));
		}

		template<typename U>
		RemoteVariableImpl<decltype(T+U)> operator+(const RemoteVariableImpl<U>& val) const
		{
			return RemoteVariableImpl<decltype(T+U)>(GetProcId(), GetValue()+val.GetValue());
		}

		template<typename U>
		RemoteVariableImpl<decltype(T-U)> operator-(const RemoteVariableImpl<U>& val) const
		{
			return RemoteVariableImpl<decltype(T-U)>(GetProcId(), GetValue()-val.GetValue());
		}

		template<typename U>
		RemoteVariableImpl& operator+=(const RemoteVariableImpl<U>& val) const
		{
			return (*this = (GetValue()+val.GetValue()));
		}

		template<typename U>
		RemoteVariableImpl& operator-=(const RemoteVariableImpl<U>& val) const
		{
			return (*this = (GetValue()-val.GetValue()));
		}
};

template<>
class RemoteVariableImpl<std::string> : public RemoteVariableImpl<const char*>
{
	public:
		RemoteVariableImpl(unsigned procId, const std::string& value) : RemoteVariableImpl<const char*>(procId, value.c_str(), static_cast<unsigned>(value.size())+1) 
		{
			Write(value.c_str(), static_cast<unsigned>(value.size())+1);
		}

		virtual bool IsArray() const { return true; }

		virtual operator std::string() const
		{
			return GetValue();
		}

		virtual RemoteVariableImpl& operator=(const std::string& val)
		{
			NewVarAddr(static_cast<unsigned>(val.size())+1);
			Write(val.c_str(), static_cast<unsigned>(val.size())+1); // +1 for the nullptr.

			return *this;
		}
};

#ifndef NO_EXTERN_REMOTE_VARIABLE_VAL

# ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4231)
# endif

extern template RemoteVariableImpl<intptr_t>;
extern template RemoteVariableImpl<uintptr_t>;
extern template RemoteVariableImpl<int64_t>;
extern template RemoteVariableImpl<uint64_t>;
extern template RemoteVariableImpl<int32_t>;
extern template RemoteVariableImpl<uint32_t>;
extern template RemoteVariableImpl<int16_t>;
extern template RemoteVariableImpl<uint16_t>;
extern template RemoteVariableImpl<int8_t>;
extern template RemoteVariableImpl<uint8_t>;
extern template RemoteVariableImpl<float>;
extern template RemoteVariableImpl<double>;

extern template RemoteVariableImpl<intptr_t*>;
extern template RemoteVariableImpl<uintptr_t*>;
extern template RemoteVariableImpl<int64_t*>;
extern template RemoteVariableImpl<uint64_t*>;
extern template RemoteVariableImpl<int32_t*>;
extern template RemoteVariableImpl<uint32_t*>;
extern template RemoteVariableImpl<int16_t*>;
extern template RemoteVariableImpl<uint16_t*>;
extern template RemoteVariableImpl<int8_t*>;
extern template RemoteVariableImpl<uint8_t*>;
extern template RemoteVariableImpl<float*>;
extern template RemoteVariableImpl<double*>;

# ifdef _MSC_VER
#  pragma warning(pop)
# endif

#endif

typedef RemoteVariableImpl<intptr_t>  RemoteSPtr;
typedef RemoteVariableImpl<uintptr_t> RemoteUPtr;
typedef RemoteVariableImpl<int64_t>	  RemoteS64;
typedef RemoteVariableImpl<uint64_t>  Remoteuint64_t;
typedef RemoteVariableImpl<int32_t>	  RemoteS32;
typedef RemoteVariableImpl<uint32_t>  RemoteU32;
typedef RemoteVariableImpl<int16_t>	  RemoteS16;
typedef RemoteVariableImpl<uint16_t>  RemoteU16;
typedef RemoteVariableImpl<int8_t>	  RemoteS8;
typedef RemoteVariableImpl<uint8_t>	  RemoteU8;
typedef RemoteVariableImpl<float>	  RemoteF32;
typedef RemoteVariableImpl<double>	  RemoteF64;

typedef RemoteVariableImpl<intptr_t*>  RemoteSPtrPtr;
typedef RemoteVariableImpl<uintptr_t*> RemoteUPtrPtr;
typedef RemoteVariableImpl<int64_t*>   RemoteS64Ptr;
typedef RemoteVariableImpl<uint64_t*>  Remoteuint64_tPtr;
typedef RemoteVariableImpl<int32_t*>   RemoteS32Ptr;
typedef RemoteVariableImpl<uint32_t*>  RemoteU32Ptr;
typedef RemoteVariableImpl<int16_t*>   RemoteS16Ptr;
typedef RemoteVariableImpl<uint16_t*>  RemoteU16Ptr;
typedef RemoteVariableImpl<int8_t*>	   RemoteS8Ptr;
typedef RemoteVariableImpl<uint8_t*>   RemoteU8Ptr;
typedef RemoteVariableImpl<float*>	   RemoteF32Ptr;
typedef RemoteVariableImpl<double*>	   RemoteF64Ptr;

typedef RemoteVariableImpl<std::string> RemoteStr;

#endif
