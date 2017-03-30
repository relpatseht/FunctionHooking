#include <iostream>
#include <cassert>
#include "FuncHookerFactory.h"
#include "TestDll.h"

void PwnFunc1(void);
void PwnFunc2(int foo);
int PwnFunc3(void);
int PwnFunc4(int foo);

auto f1 = CreateFuncHooker(TestDllFunction1, PwnFunc1);
auto f2 = CreateFuncHooker(TestDllFunction2, PwnFunc2);
auto f3 = CreateFuncHooker(TestDllFunction3, PwnFunc3);
auto f4 = CreateFuncHooker(TestDllFunction4, PwnFunc4);

void PwnFunc1(void)
{
	std::cout << "Caught f1" << std::endl;
	f1->CallInjectee();
	std::cout << "Leave f1" << std::endl;
}

void PwnFunc2(int foo)
{
	std::cout << "Caught f2 with " << foo << std::endl;
	f2->CallInjectee(foo);
	std::cout << "Leave f2" << std::endl;
}

int PwnFunc3(void)
{
	std::cout << "Caught f3" << std::endl;
	int ret = f3->CallInjectee();
	std::cout << "Left f3 with " << ret << std::endl;

	return ret;
}

int PwnFunc4(int foo)
{
	std::cout << "Caught f4 with " << foo << std::endl;
	int ret = f4->CallInjectee(foo);
	std::cout << "Left f4 with " << ret << std::endl;

	return ret;
}


int main()
{
	TestDllFunction1();
	TestDllFunction2(4);
	int ret = TestDllFunction3();
	ret = TestDllFunction4(8);

	std::cout << std::endl << std::endl;

	f1->InstallHook();
	f2->InstallHook();
	f3->InstallHook();
	f4->InstallHook();

	TestDllFunction1();
	TestDllFunction2(4);
	ret = TestDllFunction3();
	ret = TestDllFunction4(8);

	std::cout << std::endl << std::endl;

	f1->RemoveHook();
	f2->RemoveHook();
	f3->RemoveHook();
	f4->RemoveHook();

	TestDllFunction1();
	TestDllFunction2(4);
	ret = TestDllFunction3();
	ret = TestDllFunction4(8);

	std::cout << std::endl << std::endl;

	f1->InstallHook();
	f2->InstallHook();
	f3->InstallHook();
	f4->InstallHook();

	TestDllFunction1();
	TestDllFunction2(4);
	ret = TestDllFunction3();
	ret = TestDllFunction4(8);

	return 0;
}
