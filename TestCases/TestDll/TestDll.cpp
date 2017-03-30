#include <stdio.h>
#include "TestDll.h"

extern "C"
{
	void TestDllFunction1(void)
	{
		printf("You called TestDllFunction1.\n");
	}

	void TestDllFunction2(int foo)
	{
		printf("You called TestDllFunction2 with %d.\n", foo);
	}

	int TestDllFunction3(void)
	{
		printf("You called TestDllFunction3.\n");
		return 42;
	}

	int TestDllFunction4(int foo)
	{
		printf("You called TestDllFunction4 with %d.\n", foo);
		return 42;
	}

}