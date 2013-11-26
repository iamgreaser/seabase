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

	return 1;
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

