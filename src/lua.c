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
			// cud returned NULL so there SHOULD be a function there.

			// duplicate then shove into __call
			lua_getmetatable(L, -2);
			lua_pushvalue(L, -2);
			lua_pushcclosure(L, fl_call_proxy, 1);
			lua_setfield(L, -2, "__call");
			lua_setmetatable(L, -2);
			ud->ud = UD_LUA;
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

	if(ud->ud != typ && typ != UD_ANY)
		luaL_error(L, "type mismatch for %s", tname);
	
	return ud;
}


