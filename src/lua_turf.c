#include "common.h"

/**
	\brief Lua: Gets a gas/liquid type for a cell.

	\param map
	\param x
	\param y
	\param type

	\return Gas level for that given type.
*/
int fl_turf_get_gas(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 4) return luaL_error(L, "not enough arguments for turf_");

	ud_t *map_ud;
	int x, y, type;
	map_ud = ud_get_block(L, UD_MAP, "map", 1);
	x = lua_tointeger(L, 2);
	y = lua_tointeger(L, 3);
	type = lua_tointeger(L, 4);

	map_t *map = (map_t *)(map_ud->v);
	if(x < 1 || y < 1 || x > map->w || y > map->h)
		return luaL_error(L, "invalid coords %d, %d", x, y);

	if(type < 0 || type >= GAS_COUNT) return luaL_error(L, "invalid gas type %d", type);

	x--; y--;
	int mi = y*map->w + x;
	lua_pushnumber(L, map->c[mi].gas.a[type]);
	return 1;
}

/**
	\brief SQ: Sets a gas/liquid type for a cell.

	\param map
	\param x
	\param y
	\param type
	\param val
*/
int fl_turf_set_gas(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 5) return luaL_error(L, "not enough arguments for turf_");

	ud_t *map_ud;
	int x, y, type;
	double val;
	map_ud = ud_get_block(L, UD_MAP, "map", 1);
	x = lua_tointeger(L, 2);
	y = lua_tointeger(L, 3);
	type = lua_tointeger(L, 4);
	val = lua_tonumber(L, 5);

	map_t *map = (map_t *)(map_ud->v);
	if(x < 1 || y < 1 || x > map->w || y > map->h)
		return luaL_error(L, "invalid coords %d, %d", x, y);

	if(type < 0 || type >= GAS_COUNT) return luaL_error(L, "invalid gas type %d", type);

	x--; y--;
	int mi = y*map->w + x;
	map->c[mi].gas.a[type] = val;

	return 0;
}


/**
	\brief SQ: Resets the gas levels to defaults for a turf.

	\param map
	\param x
	\param y
*/
int fl_turf_reset_gas(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 3) return luaL_error(L, "not enough arguments for turf_reset_gas");

	ud_t *map_ud;
	int x, y;
	map_ud = ud_get_block(L, UD_MAP, "map", 1);
	x = lua_tointeger(L, 2);
	y = lua_tointeger(L, 3);

	map_t *map = (map_t *)(map_ud->v);
	if(x < 1 || y < 1 || x > map->w || y > map->h)
		return luaL_error(L, "invalid coords %d, %d", x, y);

	x--; y--;
	int mi = y*map->w + x;
	cell_reset_gas(&(map->c[mi]));

	return 0;
}

/**
	\brief SQ: Sets the turf type for a map.

	\param map
	\param x
	\param y
	\param type Turf type as defined by the TURF_* constants.
*/
int fl_turf_set_type(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 4) return luaL_error(L, "not enough arguments for turf_set_type");

	ud_t *map_ud;
	int x, y, type;
	map_ud = ud_get_block(L, UD_MAP, "map", 1);
	x = lua_tointeger(L, 2);
	y = lua_tointeger(L, 3);
	type = lua_tointeger(L, 4);

	map_t *map = (map_t *)(map_ud->v);
	if(x < 1 || y < 1 || x > map->w || y > map->h)
		return luaL_error(L, "invalid coords %d, %d", x, y);

	x--; y--;
	int mi = y*map->w + x;
	map->c[mi].turf.type = type;

	return 0;
}

