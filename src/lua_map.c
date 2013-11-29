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
	\brief Lua: Creates a map.

	\param w Width of map.
	\param h Height of map.

	\return Map userdata.
*/
int fl_map_new(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 2) return luaL_error(L, "not enough arguments for fl_map_new");

	int w = lua_tointeger(L, 1);
	int h = lua_tointeger(L, 2);

	if(w <= 0 || h <= 0) return luaL_error(L, "invalid map dimensions %d x %d", w, h);

	map_t *map = map_new_ud(L, w, h);
	(void)map;
	return 1;
}

/**
	\brief Lua: Perform an atmospherics simulation tick.

	\param map Map to operate on.
*/
int fl_map_tick_atmos(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for fl_map_tick_atmos");

	ud_t *ud = ud_get_block(L, UD_MAP, "map", 1);
	map_tick_atmos((map_t *)(ud->v));

	return 0;
}

