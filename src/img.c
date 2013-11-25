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
	\brief Creates a new image with a userdata block.

	\param L Lua state.
	\param w Width of image.
	\param h Height of image.

	\return New image, with userdata block pushed onto Lua stack.
*/
img_t *img_new_ud(lua_State *L, int w, int h)
{
	int dsize = (sizeof(img_t) + sizeof(uint32_t)*w*h);

	img_t *img = img_new(w, h);

	ud_t *ud = lua_newuserdata(L, sizeof(ud_t));
	ud->ud = UD_IMG;
	ud->v = (void *)img;
	ud->dlen = ud->alen = dsize;

	lua_newtable(L);
	lua_pushcfunction(L, img_free_gc);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);

	return img;
}


