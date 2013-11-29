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
		luaL_error(L, "type mismatch for %s", tname);

	if(ud->ud == UD_LOADING)
	{
		loading_t *ld = (loading_t *)(ud->v);

		if(ld->ud != typ)
			luaL_error(L, "type mismatch for %s", tname);

		// TODO: block
		fprintf(stderr, "TODO: block on UD_LOADING in ud_get_block\n");
		fflush(stderr);
		abort();
	}

	if(ud->ud != typ)
		luaL_error(L, "type mismatch for %s", tname);
	
	return ud;
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

	// TODO!
	(void)fmt;
	(void)fname;

	luaL_error(L, "TODO: common.fetch");

	return 1;
}

