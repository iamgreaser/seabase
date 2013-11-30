/*
Sea Base Omega - C Section
Copyright (C) 2013, Ben "GreaseMonkey" Russell & contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "common.h"

/**
	\brief Lua: Blocks until an object is loaded.

	\param obj Object to wait for.

	\return The object you fed in.
*/
int fl_block(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for block");

	ud_get_block(L, UD_ANY, "any", 1);

	// return input
	lua_pushvalue(L, 1);
	return 1;
}

/**
	\brief Lua: Relevant __call metamethod to remove the first userdata argument.
*/
int fl_call_proxy(lua_State *L)
{
	int top = lua_gettop(L);
	int i;

	lua_pushvalue(L, lua_upvalueindex(1));
	for(i = 2; i <= top; i++)
		lua_pushvalue(L, i);
	lua_call(L, top-1, LUA_MULTRET);

	return lua_gettop(L) - top;
}

/**
	\brief Lua: Relevant __call metamethod for Lua functions not loaded yet.
*/
int fl_block_proxy(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	ud_t *ud = ud_get_block(L, UD_LUA, "lua", lua_upvalueindex(1));

	if(ud == NULL)
		return luaL_error(L, "UD_LOADING __call has nil upvalue!");
	
	if(ud->ud != UD_LUA)
		return luaL_error(L, "UD_LOADING __call has non-UD_LUA upvalue!");
	
	// copy the args
	lua_pushvalue(L, 1);
	for(i = 2; i <= top; i++)
		lua_pushvalue(L, i);

	lua_call(L, top-1, LUA_MULTRET);
	//printf("call attempted\n");

	int ntop = lua_gettop(L);
	return ntop - top;
}

/**
	\brief Lua: Fetches a file, either locally or from the server.

	\param fmt File format to load.
	\param fname Name of file to load.

	\return Object handle, usually a userdata.
*/
int fl_fetch(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 2) return luaL_error(L, "not enough arguments for fetch");

	const char *fmt = lua_tostring(L, 1);
	const char *fname = lua_tostring(L, 2);

	printf("fetch [%s] [%s]\n", fmt, fname);

	switch(file_sec_check(fname, is_client, 0))
	{
		case SEC_LOCAL: {
			int len = 0;
			const char *data = file_get(fname, &len);

			if(data == NULL)
				return luaL_error(L, "data failed to fetch");

			file_parse_any(L, data, len, fmt, fname);
		} break;

		case SEC_REMOTE: {
			// TODO: actually send the file
			loading_t *loading = malloc(sizeof(loading_t));
			loading->ud = UD_INVALID;
			loading->v = NULL;
			loading->dlen = loading->alen = 0;

			loading->fmt = strdup(fmt);
			loading->p = loading->n = NULL; // TODO: add these to a ring
			loading->fname = strdup(fname);
			loading->resid = -1;

			ud_t *ud = lua_newuserdata(L, sizeof(ud_t));
			ud->ud = UD_LOADING;
			ud->v = (void *)loading;
			ud->dlen = ud->alen = sizeof(loading_t);

			lua_newtable(L);
			// TODO: add __gc method
			if(!strcmp(fmt, "lua"))
			{
				lua_pushvalue(L, -2);
				lua_pushcclosure(L, fl_block_proxy, 1);
				lua_setfield(L, -2, "__call");
			}
			lua_setmetatable(L, -2);
			// TODO!
			//return luaL_error(L, "TODO: remote");
		} break;

		case SEC_FORBID: {
			return luaL_error(L, "access denied");
		} break;
		
		default:
			eprintf("EDOOFUS: file_sec_check returned invalid enum!\n");
			fflush(stderr);
			abort();
	}

	return 1;
}

