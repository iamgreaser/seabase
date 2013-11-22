// i was about to call this lua.c by accident --GM

#include "common.h"

extern map_t *map_client;

/**
	\brief Lua: Creates a map.

	\param w Width of map.
	\param h Height of map.

	\return Map userpointer.
*/
int fl_map_new(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 2) return luaL_error(L, "not enough arguments for fl_map_new");

	int w = lua_tointeger(L, 1);
	int h = lua_tointeger(L, 2);

	if(w <= 0 || h <= 0) return luaL_error(L, "invalid map dimensions %d x %d", w, h);

	map_t *map = malloc(sizeof(map_t) + sizeof(cell_t)*w*h);
	map->ud = UD_MAP;
	map->w = w;
	map->h = h;

	int i;
	for(i = 0; i < w*h; i++)
	{
		cell_t *c = &(map->c[i]);
		c->turf.type = TURF_WATER;
		cell_reset_gas(c);
	}

	lua_pushlightuserdata(L, map);
	map_client = map;
	return 1;
}

