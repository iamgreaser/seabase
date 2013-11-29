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
	\brief Lua: Draws an unblended filled rectangle to an image or the screen.

	\param img Image target (nil = apply this to the screen).
	\param x1 Inclusive x boundary 1.
	\param y1 Inclusive y boundary 1.
	\param x2 Inclusive x boundary 2.
	\param y2 Inclusive y boundary 2.
	\param c Color in 0xAARRGGBB format.
*/
int fl_draw_rect_fill(lua_State *L)
{
	// XXX: should the bulk of this be in a different file + function? --GM

	int top = lua_gettop(L);
	if(top < 6) return luaL_error(L, "not enough arguments for draw_rect_fill");

	ud_t *img_ud = (lua_isnil(L, 1) ? NULL : ud_get_block(L, UD_IMG, "img", 1));
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);
	uint32_t c = (uint32_t)lua_tonumber(L, 6);
	img_t *img = (img_ud == NULL ? NULL : (img_t *)(img_ud->v));

	// Reorder
	if(x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if(y1 > y2) { int t = y1; y1 = y2; y2 = t; }

	// Get pointers and stuff
	int w = (img == NULL ? screen->w : img->w);
	int h = (img == NULL ? screen->h : img->h);
	int p = (img == NULL ? screen->pitch : 4 * img->w);
	uint8_t *d8 = (uint8_t *)(img == NULL ? screen->pixels : img->data);

	// Clip
	if(x1 < 0) x1 = 0;
	if(y1 < 0) y1 = 0;
	if(x2 >= w) x2 = w-1;
	if(y2 >= h) y2 = h-1;

	// Scissor cull
	if(x1 > x2 || y1 > y2) return 0;

	// Calculate draw info
	int bw = x2 - x1 + 1;
	int bh = y2 - y1 + 1;
	d8 += p*y1 + 4*x1;

	// Draw!
	int x, y;
	for(y = 0; y < bh; y++)
	{
		uint32_t *d32 = (uint32_t *)d8;

		for(x = 0; x < bw; x++)
			*(d32++) = c;

		d8 += p;
	}

	// Return
	return 0;
}

/**
	\brief Lua: Draws an unblended outlined rectangle to an image or the screen.

	\param img Image target (nil = apply this to the screen).
	\param x1 Inclusive x boundary 1.
	\param y1 Inclusive y boundary 1.
	\param x2 Inclusive x boundary 2.
	\param y2 Inclusive y boundary 2.
	\param c Color in 0xAARRGGBB format.
*/
int fl_draw_rect_outl(lua_State *L)
{
	// XXX: should the bulk of this be in a different file + function? --GM

	int top = lua_gettop(L);
	if(top < 6) return luaL_error(L, "not enough arguments for draw_rect_fill");

	ud_t *img_ud = (lua_isnil(L, 1) ? NULL : ud_get_block(L, UD_IMG, "img", 1));
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);
	uint32_t c = (uint32_t)lua_tonumber(L, 6);
	img_t *img = (img_ud == NULL ? NULL : (img_t *)(img_ud->v));

	// Reorder
	if(x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if(y1 > y2) { int t = y1; y1 = y2; y2 = t; }

	// Get pointers and stuff
	int w = (img == NULL ? screen->w : img->w);
	int h = (img == NULL ? screen->h : img->h);
	int p = (img == NULL ? screen->pitch : 4 * img->w);
	uint8_t *d8 = (uint8_t *)(img == NULL ? screen->pixels : img->data);

	// Clip
	if(x1 < 0) x1 = 0;
	if(y1 < 0) y1 = 0;
	if(x2 >= w) x2 = w-1;
	if(y2 >= h) y2 = h-1;

	// Scissor cull
	if(x1 > x2 || y1 > y2) return 0;

	// Calculate draw info
	int bw = x2 - x1 + 1;
	int bh = y2 - y1 + 1;

	// Draw horiz
	{
		int x;

		uint32_t *d0 = (uint32_t *)(d8 + p*y1 + 4*x1);
		uint32_t *d1 = (uint32_t *)(d8 + p*y2 + 4*x1);

		for(x = 0; x < bw; x++)
			*(d0++) = *(d1++) = c;
	}

	// Draw vert
	{
		int y;

		uint8_t *d0a = (d8 + p*y1 + 4*x1);
		uint8_t *d1a = (d8 + p*y1 + 4*x2);

		for(y = 0; y < bh; y++)
		{
			*(uint32_t *)d0a = *(uint32_t *)d1a = c;
			d0a += p;
			d1a += p;
		}
	}

	// Return
	return 0;
}


