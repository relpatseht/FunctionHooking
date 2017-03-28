/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		ArgTypeTraits.h
 *  \author		Andrew Shurney
 *  \brief		Simple type traits library
 */

/* All Content © 2011 DigiPen (USA) Corporation, all rights reserved.              */

#ifndef ARG_TYPE_TRAITS_H
#define ARG_TYPE_TRAITS_H

#ifdef _MSC_VER
#pragma warning(disable: 6285)
#endif

template<typename T>
struct IsPointer
{
	static const bool value = false;
};

template<typename T>
struct IsPointer<T*>
{
	static const bool value = true;
};

template<typename T>
struct IsReference
{
	static const bool value = false;
};

template<typename T>
struct IsReference<T&>
{
	enum
	{
		value = 1
	};
};

template<typename T>
struct IsByValue
{
	static const bool value = !(IsReference<T>::value || IsPointer<T>::value);
};

template<typename T, bool ByValue=false>
struct _FastIntermediaryHelper
{
    typedef T type;
};

template<typename T>
struct _FastIntermediaryHelper<T, true>
{
    typedef const T& type;
};

template<typename T>
struct FastIntermediary
{
	public:
		typedef typename _FastIntermediaryHelper<T, IsByValue<T>::value>::type type;
};

#endif
