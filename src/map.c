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

