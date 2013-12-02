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

SDL_Surface *real_screen;
SDL_Surface *screen;
lua_State *L_client;
lua_State *L_server;
map_t *map_server = NULL;
int64_t time_current, time_prev;

int is_client = 1;

/**
	\brief Prints error info to stderr.

	\param fmt Format string.
*/
void eprintf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

#ifdef WIN32
	// Fuck you Windows.
	vprintf(fmt, va);
#else
	vfprintf(stderr, fmt, va);
#endif

	va_end(va);
}

/**
	\brief Get the current time in microseconds.

	\return Current time in microseconds.
*/
int64_t time_get_us(void)
{
	// TODO: Microsoft Make-it-hard-for-everyone-else-dows version
	struct timeval tv;
	gettimeofday(&tv, NULL);

	int64_t s = (int64_t)tv.tv_sec;
	int64_t u = (int64_t)tv.tv_usec;
	s *= (int64_t)1000000;
	u += s;

	return u;
}

/**
	\brief Lua: wrapped loadfile

	\param fname

	\return Lua function, or a Lua error on failure.
*/
int fl_wrap_loadfile(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for loadfile");

	const char *fname = lua_tostring(L, 1);
	lua_getglobal(L, "common");
	lua_getfield(L, -1, "fetch");
	lua_remove(L, -2);
	lua_pushstring(L, "lua");
	lua_pushstring(L, fname);
	lua_call(L, 2, 1);

	return 1;
}

/**
	\brief Lua: wrapped dofile

	\param fname

	\return Result of argumentless call to function loaded by loadfile.
*/
int fl_wrap_dofile(lua_State *L)
{
	int top = lua_gettop(L);
	if(top < 1) return luaL_error(L, "not enough arguments for dofile");

	lua_getglobal(L, "loadfile");
	lua_pushvalue(L, 1);
	lua_call(L, 1, 1);
	lua_call(L, 0, LUA_MULTRET);

	int ntop = lua_gettop(L);
	return ntop - top;
}

/**
	\brief Loads properly wrapped base Lua libraries.

	\param L Lua state.
*/
void open_libs_lua(lua_State *L)
{
	// Load libraries
	lua_pushcfunction(L, luaopen_base); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_math); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_table); lua_call(L, 0, 0);

	// Wrap certain functions
	// TODO: forbid bytecode in loadstring(s) / load(fn[, chunkname])
	lua_pushcfunction(L, fl_wrap_dofile); lua_setglobal(L, "dofile");
	lua_pushcfunction(L, fl_wrap_loadfile); lua_setglobal(L, "loadfile");
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	time_current = time_prev = time_get_us();

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	atexit(SDL_Quit);

	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	L_server = luaL_newstate();
	L_client = luaL_newstate();

	// TODO: restrict the libraries loaded to a subset
	printf("Loading functions\n");
	open_libs_lua(L_server);
	open_libs_lua(L_client);

#define ADDFN_L(L, f, s) \
		lua_pushcfunction(L, f); \
		lua_pushvalue(L, -1); \
		lua_setfield(L, -3, s); \
		lua_setfield(L, -3, s);

	// client/server tables
	lua_newtable(L_client);
	lua_newtable(L_server);

	// common functions
#define ADDFN(f, s) \
		ADDFN_L(L_client, f, s); \
		ADDFN_L(L_server, f, s);
	lua_newtable(L_client);
	lua_newtable(L_server);

	ADDFN(fl_fetch, "fetch");
	ADDFN(fl_block, "block");

	ADDFN(fl_draw_rect_fill, "draw_rect_fill");
	ADDFN(fl_draw_rect_outl, "draw_rect_outl");

	ADDFN(fl_img_new, "img_new");
	ADDFN(fl_img_get_dims, "img_get_dims");
	ADDFN(fl_img_get_pixel, "img_get_pixel");
	ADDFN(fl_img_blit, "img_blit");

	ADDFN(fl_map_new, "map_new");
	ADDFN(fl_map_tick_atmos, "map_tick_atmos");

	ADDFN(fl_mouse_get, "mouse_get");

	ADDFN(fl_turf_get_type, "turf_get_type");
	ADDFN(fl_turf_set_type, "turf_set_type");
	ADDFN(fl_turf_reset_gas, "turf_reset_gas");
	ADDFN(fl_turf_get_gas, "turf_get_gas");
	ADDFN(fl_turf_set_gas, "turf_set_gas");
	lua_setglobal(L_client, "common");
	lua_setglobal(L_server, "common");
#undef ADDFN
	lua_setglobal(L_client, "client");
	lua_setglobal(L_server, "server");
	printf("Functions loaded\n");

	SDL_WM_SetCaption("Sea Base Omega - 0.0 prealpha", NULL);
	real_screen = SDL_SetVideoMode(960, 600, 32, 0);
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 32,
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	lua_getglobal(L_client, "common");
	lua_getfield(L_client, -1, "fetch");
	lua_remove(L_client, -2);
	lua_pushstring(L_client, "lua");
	lua_pushstring(L_client, "pkg/base/main_client.lua");
	lua_call(L_client, 2, 1);
	if(1)
	{
		// TODO: use pcall
		lua_call(L_client, 0, 0);
	} else {
		printf("File failed to compile: %s\n", lua_tostring(L_client, -1));
	}

	printf("init done\n");

	int quitflag = 0;
	while(!quitflag)
	{
		int x, y;
		// clear screen
		memset(screen->pixels, 0, screen->pitch*screen->h);

		// update timer
		time_prev = time_current;
		time_current = time_get_us();

		// TODO: use pcall
		lua_getglobal(L_client, "hook_tick");
		if(!lua_isnil(L_client, -1))
		{
			lua_pushnumber(L_client, ((double)time_current)/1000000.0);
			lua_pushnumber(L_client, ((double)(time_current - time_prev))/1000000.0);
			lua_call(L_client, 2, 0);
		} else {
			printf("hook_tick DNE\n");
			lua_pop(L_client, 1);
		}

		// TODO: use pcall
		lua_getglobal(L_client, "hook_render");
		if(!lua_isnil(L_client, -1))
		{
			lua_pushnumber(L_client, ((double)time_current)/1000000.0);
			lua_pushnumber(L_client, ((double)(time_current - time_prev))/1000000.0);
			lua_call(L_client, 2, 0);
		} else {
			printf("hook_render DNE\n");
			lua_pop(L_client, 1);
		}

		// manual 3x upscaling of image
		for(y = 0; y < 200; y++)
		{
			uint32_t *d0 = (uint32_t *)(real_screen->pixels + (y*3+0)*real_screen->pitch);
			uint32_t *d1 = (uint32_t *)(real_screen->pixels + (y*3+1)*real_screen->pitch);
			uint32_t *d2 = (uint32_t *)(real_screen->pixels + (y*3+2)*real_screen->pitch);
			uint32_t *s = (uint32_t *)(screen->pixels + y*screen->pitch);

			for(x = 0; x < 320; x++)
			{
				*(d0++) = *(d1++) = *(d2++) = *s;
				*(d0++) = *(d1++) = *(d2++) = *s;
				*(d0++) = *(d1++) = *(d2++) = *(s++);
			}
		}

		SDL_Flip(real_screen);

		SDL_Delay(10);

		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_QUIT:
				quitflag = 1;
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				input_mouse_update_button(L_client,
					ev.button.x, ev.button.y,
					ev.button.button - 1,
					ev.type == SDL_MOUSEBUTTONDOWN);
				break;
			case SDL_MOUSEMOTION:
				input_mouse_update_pos(L_client,
					ev.motion.x, ev.motion.y);
				break;
		}
	}

	return 0;
}

