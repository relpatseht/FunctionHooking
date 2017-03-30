#include "FuncHookerFactory.h"
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
# pragma warning(disable:4091)
# include <DbgHelp.h>
# pragma warning(default:4091)
#include <iostream>

extern "C"
{
	#include "lstate.h"
	void *luaStateHookPtr = NULL;
	std::vector<lua_State*> luaStates;

	static lua_State *MyLuaNewState (lua_Alloc f, void *ud)
	{
		auto luaStateHook = FuncHookerCast(luaStateHookPtr, MyLuaNewState);

		lua_State *state = luaStateHook->CallInjectee(f, ud);
		luaStates.push_back(state);

		std::cout << "Got a state!" << std::endl;

		return state;
	}

	extern __declspec(dllexport) bool InstallHooks()
	{
		auto luaStateHook = CreateFuncHooker("lua_newstate", MyLuaNewState);
		if(!luaStateHook)
			return false;

		if(luaStateHook->InstallHook())
		{
			luaStateHookPtr = luaStateHook;
			return true;
		}

		return false;
	}
}
