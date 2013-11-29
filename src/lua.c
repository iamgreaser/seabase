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
// i was about to call this lua.c by accident --GM

// there is a very good reason why the above comment exists
// although there is no good reason why it STILL exists --GM

#include "common.h"

/**
	\brief Lua helper: Gets a userdata safely, blocking if data still loading.

	\param L Lua state.
	\param typ Expected userdata type.
	\param tname Name of userdata type.
	\param idx Index of variable to get userdata from.

	\return Userdata if successful and matching; throws Lua error otherwise.
*/
ud_t *ud_get_block(lua_State *L, int typ, char *tname, int idx)
{
	ud_t *ud = lua_touserdata(L, idx);

	if(ud == NULL)
		luaL_error(L, "got a null for %s", tname);

	if(ud->ud == UD_LOADING)
	{
		loading_t *loading = (loading_t *)(ud->v);

		// TODO: actually use the correct ud
		//if(loading->ud != typ)
		//	luaL_error(L, "type mismatch for %s", tname);

		// push current userdata onto the stack
		// this is just in case someone gave idx a -ve value
		lua_pushvalue(L, idx);

		// TODO: actually block
		// FIXME: this should use ud_get_block, actually
		// get data
		{
			// TODO: block nicely when we actually network stuff
			int len = 0;
			const char *data = file_get(loading->fname, &len);

			if(data == NULL)
				luaL_error(L, "data failed to fetch");

			file_parse_any(L, data, len, loading->fmt, loading->fname);
		}

		// delete the loading_t structure
		if(loading->v != NULL) free(loading->v);
		if(loading->n != NULL) ((loading_t *)(loading->n->v))->p = loading->p;
		if(loading->p != NULL) ((loading_t *)(loading->p->v))->n = loading->n;
		free(loading->fname);
		free(loading->fmt);

		// steal everything from the new userdata (if possible)
		ud_t *cud = lua_touserdata(L, -1);

		if(cud == NULL)
		{
			fprintf(stderr, "TODO: ud_get_block blocked for Lua scripts\n");
			fflush(stderr);
			abort();
		} else {
			// this sort of shit would make the average Java programmer cry.
			// it also makes us C programmers glad that we use C :)
			memcpy(ud, cud, sizeof(ud_t));
			lua_getmetatable(L, -1);
			lua_setmetatable(L, -3);
			lua_newtable(L);
			lua_setmetatable(L, -2);
			cud->ud = UD_INVALID;
			cud->v = NULL;
		}

		// remove cud from the stack
		lua_remove(L, -1);

		// also remove ud from the stack
		lua_remove(L, -1);
	}

	if(ud->ud != typ)
		luaL_error(L, "type mismatch for %s", tname);
	
	return ud;
}

/**
	\brief Lua: Blocks until an object is loaded.

	\param obj Object to wait for.

	\return The object you fed in.
*/
int fl_block(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for block");

	luaL_error(L, "TODO: common.block");

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
	ud_t *ud = lua_touserdata(L, lua_upvalueindex(1));

	if(ud == NULL)
		return luaL_error(L, "UD_LOADING __call has nil upvalue!");
	
	if(ud->ud != UD_LOADING)
		return luaL_error(L, "UD_LOADING __call has non-UD_LOADING upvalue!");
	
	loading_t *loading = (loading_t *)(ud->v);

	// get function
	{
		// TODO: block nicely when we actually network stuff
		int len = 0;
		const char *data = file_get(loading->fname, &len);

		if(data == NULL)
			return luaL_error(L, "data failed to fetch");

		file_parse_any(L, data, len, loading->fmt, loading->fname);
	}

	// duplicate then shove into __call
	lua_getmetatable(L, lua_upvalueindex(1));
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, fl_call_proxy, 1);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, lua_upvalueindex(1));

	// delete the loading_t structure
	if(loading->v != NULL) free(loading->v);
	if(loading->n != NULL) ((loading_t *)(loading->n->v))->p = loading->p;
	if(loading->p != NULL) ((loading_t *)(loading->p->v))->n = loading->n;
	free(loading->fname);
	free(loading->fmt);

	ud->ud = UD_LUA;
	ud->v = NULL;
	ud->dlen = ud->alen = 0;

	// copy the args
	for(i = 1; i < top; i++)
		lua_pushvalue(L, i+1);

	//printf("attempt call %i\n", top);
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
			fprintf(stderr, "EDOOFUS: file_sec_check returned invalid enum!\n");
			fflush(stderr);
			abort();
	}

	return 1;
}

