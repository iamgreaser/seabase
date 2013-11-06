// i was about to call this lua.c by accident --GM

#include "common.h"

extern map_t *map_client;

/**
	\brief SQ: Resets the gas levels to defaults for a turf.

	\param map
	\param x
	\param y
*/
SQInteger fsq_turf_reset_gas(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);
	if(top != 4) return SQ_ERROR;

	SQUserPointer map_ud;
	SQInteger x, y, type;
	if(SQ_FAILED(sq_getuserpointer(S, 2, &map_ud))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 3, &x))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 4, &y))) return SQ_ERROR;

	map_t *map = (map_t *)map_ud;
	if(map->ud != UD_MAP) return SQ_ERROR;
	if(x < 0) return SQ_ERROR;
	if(y < 0) return SQ_ERROR;
	if(x >= map->w) return SQ_ERROR;
	if(y >= map->h) return SQ_ERROR;

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
SQInteger fsq_turf_set_type(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);
	if(top != 5) return SQ_ERROR;

	SQUserPointer map_ud;
	SQInteger x, y, type;
	if(SQ_FAILED(sq_getuserpointer(S, 2, &map_ud))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 3, &x))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 4, &y))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 5, &type))) return SQ_ERROR;

	map_t *map = (map_t *)map_ud;
	if(map->ud != UD_MAP) return SQ_ERROR;
	if(x < 0) return SQ_ERROR;
	if(y < 0) return SQ_ERROR;
	if(x >= map->w) return SQ_ERROR;
	if(y >= map->h) return SQ_ERROR;

	int mi = y*map->w + x;
	map->c[mi].turf.type = type;

	return 0;
}

/**
	\brief SQ: Creates a map.

	\param w Width of map.
	\param h Height of map.

	\return Map userpointer.
*/
SQInteger fsq_map_new(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);
	if(top != 3) return SQ_ERROR;

	SQInteger w, h;
	if(SQ_FAILED(sq_getinteger(S, 2, &w))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 3, &h))) return SQ_ERROR;

	if(w <= 0) return SQ_ERROR;
	if(h <= 0) return SQ_ERROR;

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

	sq_pushuserpointer(S, map);
	map_client = map;
	return 1;
}

