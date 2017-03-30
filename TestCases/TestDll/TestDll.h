#ifndef TEST_DLL_H
#define TEST_DLL_H

#if defined(WIN32) || defined(WIN64)
# ifdef BUILDING_DLL
#  define TESTDLL_DLLAPI __declspec(dllexport)
# else
#  define TESTDLL_DLLAPI __declspec(dllimport)
# endif
# define TESTDLL_DLLCALL __cdecl
#else
# define TESTDLL_DLLAPI extern
# define TESTDLL_DLLCALL 
#endif

extern "C"
{
	TESTDLL_DLLAPI void TESTDLL_DLLCALL TestDllFunction1(void);
	TESTDLL_DLLAPI void TESTDLL_DLLCALL TestDllFunction2(int foo);
	TESTDLL_DLLAPI int  TESTDLL_DLLCALL TestDllFunction3(void);
	TESTDLL_DLLAPI int  TESTDLL_DLLCALL TestDllFunction4(int foo);
}

#undef TESTDLL_DLLAPI
#undef TESTDLL_DLLCALL

#endif
