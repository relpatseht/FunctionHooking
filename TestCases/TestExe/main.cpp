#include "lua.hpp"
#include <iostream>
#include <windows.h>

int main(int argc, char *argv[])
{
	lua_State *l = luaL_newstate();

	luaL_openlibs(l);

	system("pause");

	if(argc < 2)
	{
		std::cout << "Usage: TestExe <file>.lua" << std::endl;
		return -1;
	}

	int s = luaL_loadfile(l, argv[1]);
	if(!s)
	{
		s = lua_pcall(l, 0, LUA_MULTRET, 0);
		if(s)
		{
			std::cout << "[E] " << lua_tostring(l, -1) << std::endl;
			lua_pop(l, 1);
		}
	}
	else
	{
		std::cout << "[E] " << lua_tostring(l, -1) << std::endl;
		lua_pop(l, 1);
	}

	lua_close(l);
	return 0;
}
