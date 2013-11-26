// i was about to call this lua.c by accident --GM

// there is a very good reason why the above comment exists
// although there is no good reason why it STILL exists --GM

#include "common.h"

extern map_t *map_client;

/**
	\brief Lua helper: Gets a userdata safely, blocking if data still loading.

	\param L Lua state.
	\param typ Expected userdata type.
	\param tname Name of userdata type.
	\param idx Index of variable to get userdata from.

	\return Userdata if successful and matching; throws Lua error otherwise.
*/
ud_t *ud_get_block(lua_State *L, int typ, char *tname, int idx)
{
	ud_t *ud = lua_touserdata(L, idx);

	if(ud == NULL)
		luaL_error(L, "type mismatch for %s", tname);

	if(ud->ud == UD_LOADING)
	{
		loading_t *ld = (loading_t *)(ud->v);

		if(ld->ud != typ)
			luaL_error(L, "type mismatch for %s", tname);

		// TODO: block
		fprintf(stderr, "TODO: block on UD_LOADING in ud_get_block\n");
		fflush(stderr);
		abort();
	}

	if(ud->ud != typ)
		luaL_error(L, "type mismatch for %s", tname);
	
	return ud;
}

/**
	\brief Lua: Creates a blank image.

	\param w Width of image.
	\param h Height of image.

	\return Image userpointer.
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
	\brief Lua: Loads an image from a file, throwing an error on failure.

	\param fname Filename of image to load.
	\param fmt File format (default: "png").

	\return Image userpointer.
*/
int fl_img_load(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for img_load");

	const char *fname = lua_tostring(L, 1);
	const char *fmt = (top < 2 ? "png" : lua_tostring(L, 1));

	// TODO: call common.fetch when it exists
	(void)fmt;

	int len = 0;
	const char *data = file_get(fname, &len);
	if(data == NULL)
		return luaL_error(L, "image failed to fetch");
	
	// TODO: handle when format != "png"
	img_t *img = img_load_png(data, len);
	if(img == NULL)
		return luaL_error(L, "image failed to parse");
	ud_t *ud = img_provide_ud(L, img);
	(void)ud;
	return 1;
}

/**
	\brief Lua: Blits from an image/the screen to an image/the screen.
	
	Blitting to/from the screen on the server will throw a Lua error.

	WARNING: When blitting to/from the same image/screen,
	the rectangles MUST NOT OVERLAP; otherwise, behaviour is UNDEFINED.

	\param fname Filename of image to load.
	\param fmt File format (default: "png").

	\return Image userpointer.
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
		dest_img_ud = ud_get_block(L, UD_IMG, "img", 1);
	
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

	map_t *map = map_new_ud(L, w, h);
	map_client = map;
	return 1;
}

