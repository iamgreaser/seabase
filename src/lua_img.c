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
	\brief Lua: Creates a blank image.

	\param w Width of image.
	\param h Height of image.

	\return Image userdata.
*/
int fl_img_new(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 2) return luaL_error(L, "not enough arguments for img_new");

	int w = lua_tointeger(L, 1);
	int h = lua_tointeger(L, 2);

	if(w <= 0 || h <= 0) return luaL_error(L, "invalid img dimensions %d x %d", w, h);

	img_t *img = img_new_ud(L, w, h);
	(void)img;
	return 1;
}

/**
	\brief Lua: Gets an image's dimensions.

	\param img Image.

	\return w, h dimensions.
*/
int fl_img_get_dims(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for img_get_dims");

	if(lua_isnil(L, 1))
	{
		lua_pushinteger(L, screen->w);
		lua_pushinteger(L, screen->h);
		return 2;
	}

	ud_t *img_ud = ud_get_block(L, UD_IMG, "img", 1);
	img_t *img = (img_t *)img_ud;
	
	lua_pushinteger(L, img->w);
	lua_pushinteger(L, img->h);
	return 2;
}
/**
	\brief Lua: Reads a pixel from an image.

	\param img Image to read from.
	\param x Coordinate.
	\param y Coordinate.
	\param def Default return value for out of range pixels. Defaults to 0x00000000.
		Can be absolutely anything.

	\returns ARGB pixel data as an integer, or argument "def" if out of range.
*/
int fl_img_get_pixel(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 3) return luaL_error(L, "not enough arguments for img_get_pixel");

	ud_t *img_ud = NULL;

	if(!lua_isnil(L, 1))
		img_ud = ud_get_block(L, UD_IMG, "img", 1);
	
	img_t *img = (img_ud == NULL ? NULL : (img_t *)(img_ud->v));
	
	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);
	int w = (img == NULL ? screen->w : img->w);
	int h = (img == NULL ? screen->h : img->h);

	if(x >= 0 && x < w && y >= 0 && y < h)
	{
		if(img == NULL)
		{
			uint8_t *pty = ((uint8_t *)(screen->pixels)) + y*screen->pitch;
			uint32_t *ptx = ((uint32_t *)pty) + x;
			lua_pushinteger(L, *ptx);
		} else {
			lua_pushinteger(L, img->data[y*w + x]);
		}
	} else {
		if(top < 4) lua_pushinteger(L, 0x00000000);
		else lua_pushvalue(L, 4);
	}

	return 1;
}

/**
	\brief Lua: Blits from an image/the screen to an image/the screen.
	
	Blitting to/from the screen on the server will throw a Lua error.

	WARNING: When blitting to/from the same image/screen,
	the rectangles MUST NOT OVERLAP; otherwise, behaviour is UNDEFINED.

	See docs/lua_api.txt for the parameters; or better still, look at the source ;)
*/
int fl_img_blit(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 4) return luaL_error(L, "not enough arguments for img_blit");

	ud_t *src_img_ud = NULL;

	if(!lua_isnil(L, 1))
		src_img_ud = ud_get_block(L, UD_IMG, "img", 1);
	
	img_t *src_img = (src_img_ud == NULL ? NULL : (img_t *)(src_img_ud->v));
	
	int dx = lua_tointeger(L, 2);
	int dy = lua_tointeger(L, 3);
	int flags = lua_tointeger(L, 4);

	int sx = (top < 5 ? 0 : lua_tointeger(L, 5));
	int sy = (top < 6 ? 0 : lua_tointeger(L, 6));
	int bw = (top < 7 ? (src_img == NULL ? screen->w : src_img->w) : lua_tointeger(L, 7));
	int bh = (top < 8 ? (src_img == NULL ? screen->h : src_img->h) : lua_tointeger(L, 8));

	ud_t *dest_img_ud = NULL;
	if(top >= 9 && !lua_isnil(L, 9))
		dest_img_ud = ud_get_block(L, UD_IMG, "img", 9);
	
	//printf("lua blit %i %i %i %i %i %i\n", sx, sy, dx, dy, bw, bh);
	if(src_img_ud == NULL)
	{
		if(dest_img_ud == NULL)
		{
			blit_sdl_to_sdl(
				screen, sx, sy,
				screen, dx, dy,
				bw, bh, flags);
		} else {
			blit_sdl_to_img(
				screen, sx, sy,
				(img_t *)(dest_img_ud->v), dx, dy,
				bw, bh, flags);
		}
	} else {
		if(dest_img_ud == NULL)
		{
			blit_img_to_sdl(
				(img_t *)(src_img_ud->v), sx, sy,
				screen, dx, dy,
				bw, bh, flags);
		} else {
			blit_img_to_img(
				(img_t *)(src_img_ud->v), sx, sy,
				(img_t *)(dest_img_ud->v), dx, dy,
				bw, bh, flags);
		}
	}
	
	return 0;
}

