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
	\brief Resets the gas for a cell according to the turf type.

	\param c Cell to reset.
*/
void cell_reset_gas(cell_t *c)
{
	c->gas.g.vx = c->gas.g.vy = 0.0f;

	c->gas.g.co2 = 0.0f;
	c->gas.g.ch4 = 0.0f;
	c->gas.g.water = 0.0f;
	c->gas.g.o2 = 0.0f;
	c->gas.g.n2 = 0.0f;
	switch(c->turf.type)
	{
		case TURF_WATER:
			c->gas.g.water = 1.0f;
			break;
		case TURF_FLOOR:
			c->gas.g.o2 = 0.2f;
			c->gas.g.n2 = 0.8f;
			break;
		default:
			break;
	}
}

/**
	\brief Frees a map.

	\param map Map to free.
*/
void map_free(map_t *map)
{
	free((void *)map);
}

int map_free_gc(lua_State *L)
{
	ud_t *ud = lua_touserdata(L, 1);
	printf("Freeing map %p\n", ud);
	map_free((map_t *)(ud->v));
	return 0;
}

/**
	\brief Creates a new map.

	\param w Width of map.
	\param h Height of map.

	\return New map.
*/
map_t *map_new(int w, int h)
{
	int i;
	int dsize = (sizeof(map_t) + sizeof(cell_t)*w*h);

	map_t *map = malloc(dsize);
	map->w = w;
	map->h = h;

	// Initialise map data
	for(i = 0; i < w*h; i++)
	{
		cell_t *c = &(map->c[i]);
		c->turf.type = TURF_WATER;
		cell_reset_gas(c);
	}

	return map;
}

/**
	\brief Creates a new map with a userdata block.

	\param L Lua state.
	\param w Width of map.
	\param h Height of map.

	\return New map, with userdata block pushed onto Lua stack.
*/
map_t *map_new_ud(lua_State *L, int w, int h)
{
	int dsize = (sizeof(map_t) + sizeof(cell_t)*w*h);

	map_t *map = map_new(w, h);
	ud_t *ud = lua_newuserdata(L, sizeof(ud_t));
	ud->ud = UD_MAP;
	ud->v = (void *)map;
	ud->dlen = ud->alen = dsize;

	lua_newtable(L);
	lua_pushcfunction(L, map_free_gc);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);

	return map;
}

/**
	\brief Perform an atmospherics simulation tick.

	\param map Map to operate on.
*/
void map_tick_atmos(map_t *map)
{
	//
	int x, y, i;
	cell_t edge;
	edge.turf.type = TURF_WATER;
	cell_reset_gas(&edge);
	edge.gas_old.g = edge.gas.g;

	for(y = 0; y < map->h; y++)
	for(x = 0; x < map->w; x++)
	{
		cell_t *c = &(map->c[y * map->w + x]);
		c->gas_old.g = c->gas.g;
	}

	for(y = 0; y < map->h; y++)
	for(x = 0; x < map->w; x++)
	{
		// current cell
		cell_t *c = &(map->c[y * map->w + x]);

		// walls don't leak
		if(c->turf.type == TURF_WALL)
			continue;

		// neighbours
		cell_t *n[4];
		n[0] = (x == 0 ? &edge : c-1);
		n[1] = (y == 0 ? &edge : c-map->w);
		n[2] = (x+1 == map->w ? &edge : c+1);
		n[3] = (y+1 == map->h ? &edge : c+map->w);

		// calculate cell gas proportions
		float dx = 0.0f;
		float dy = 0.0f;
		for(i = 0; i < GAS_COUNT; i++)
		{
			float sdx = 0.0f;
			float sdy = 0.0f;
			float samt = 0.0f;

			float gstr = 0.25f;

			c->gas.a[i] = c->gas_old.a[i];

			if(n[0]->turf.type != TURF_WALL)
			{
				sdx -= gstr*(n[0]->gas_old.a[i] - c->gas_old.a[i]);
				samt += gstr*(n[0]->gas_old.a[i] - c->gas_old.a[i]);
			}

			if(n[1]->turf.type != TURF_WALL)
			{
				sdy -= gstr*(n[1]->gas_old.a[i] - c->gas_old.a[i]);
				samt += gstr*(n[1]->gas_old.a[i] - c->gas_old.a[i]);
			}

			if(n[2]->turf.type != TURF_WALL)
			{
				sdx += gstr*(n[2]->gas_old.a[i] - c->gas_old.a[i]);
				samt += gstr*(n[2]->gas_old.a[i] - c->gas_old.a[i]);
			}

			if(n[3]->turf.type != TURF_WALL)
			{
				sdy += gstr*(n[3]->gas_old.a[i] - c->gas_old.a[i]);
				samt += gstr*(n[3]->gas_old.a[i] - c->gas_old.a[i]);
			}

			c->gas.a[i] += samt;
			dx += sdx;
			dy += sdy;
		}

		// balance water if TURF_WATER
		if(c->turf.type == TURF_WATER)
		{
			c->gas.g.water += 0.3f*(1.0f - c->gas.g.water);
			for(i = 1; i < GAS_COUNT; i++)
				c->gas.a[i] += 0.3f*(0.0f - c->gas.a[i]);
		}

		c->gas.g.vx = dx;
		c->gas.g.vy = dy;
	}
}

