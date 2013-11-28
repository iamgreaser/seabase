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
	\brief Frees an image.

	\param img Image to free.
*/
void img_free(img_t *img)
{
	free((void *)img);
}

int img_free_gc(lua_State *L)
{
	ud_t *ud = lua_touserdata(L, 1);
	printf("Freeing img %p\n", ud);
	img_free((img_t *)(ud->v));
	return 0;
}

/**
	\brief Creates a new, blank image.

	\param w Width of image.
	\param h Height of image.

	\return New image.
*/
img_t *img_new(int w, int h)
{
	int i;
	int dsize = (sizeof(img_t) + sizeof(uint32_t)*w*h);

	img_t *img = malloc(dsize);

	img->w = w;
	img->h = h;
	for(i = 0; i < w*h; i++)
		img->data[i] = 0x00000000; // fully transparent, black

	return img;
}

/**
	\brief Creates a userdata block for an image.

	\param L Lua state.
	\param img Image.

	\return Userdata block as pushed onto the Lua stack.
*/
ud_t *img_provide_ud(lua_State *L, img_t *img)
{
	int dsize = (sizeof(img_t) + sizeof(uint32_t) * (img->w) * (img->h));

	ud_t *ud = lua_newuserdata(L, sizeof(ud_t));
	ud->ud = UD_IMG;
	ud->v = (void *)img;
	ud->dlen = ud->alen = dsize;

	lua_newtable(L);
	lua_pushcfunction(L, img_free_gc);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);

	return ud;
}

/**
	\brief Creates a new image with a userdata block.

	\param L Lua state.
	\param w Width of image.
	\param h Height of image.

	\return New image, with userdata block pushed onto Lua stack.
*/
img_t *img_new_ud(lua_State *L, int w, int h)
{
	img_t *img = img_new(w, h);

	img_provide_ud(L, img);

	return img;
}

