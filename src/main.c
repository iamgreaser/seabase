/*
i'll sort out the licensing crap when i get around to it --GM
*/

#include "common.h"

SDL_Surface *real_screen;
SDL_Surface *screen;
lua_State *L_client;
lua_State *L_server;
map_t *map_client = NULL;
map_t *map_server = NULL;

int bubcount = 0;

/*
	// brief Simple stdout print function for the Squirrel VM.
	Might be useful later?
*/
/*
void hsq_print_stdout(HSQUIRRELVM S, const SQChar *buf, ...)
{
	va_list vl;

	printf((S == S_server ? "[server] " : S == S_client ? "[client] " : "[??????] "));

	va_start(vl, buf);
	vprintf(buf, vl);
	va_end(vl);

	printf("\n");
}
*/

uint8_t dithtab2[4] = {
	0, 2,
	3, 1,
};

uint8_t dithtab4[16] = {
	0x0, 0x8, 0x2, 0xA,
	0xC, 0x4, 0xE, 0x6,
	0x3, 0xB, 0x1, 0x9,
	0xF, 0x7, 0xD, 0x5,
};

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
	// TODO: loadfile(fname) -> return common.fetch("lua", fname)
	// TODO: forbid bytecode in loadstring(s) / load(fn[, chunkname])
	lua_pushcfunction(L, fl_wrap_dofile); lua_setglobal(L, "dofile");
}

int main(int argc, const char *argv)
{
	char *data;
	int len;
	data = file_get_direct("pkg/base/gfx/hello.png", &len);
	img_load_png(data, len);
	
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

	ADDFN(fl_img_new, "img_new");

	ADDFN(fl_map_new, "map_new");
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

	if(luaL_loadfile(L_client, "pkg/base/main_client.lua") == 0)
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
		memset(screen->pixels, 0, screen->pitch*screen->h);

		// TODO: use pcall
		lua_getglobal(L_client, "hook_tick");
		// TODO: give actual times
		if(!lua_isnil(L_client, -1))
		{
			lua_pushnumber(L_client, 0.0);
			lua_pushnumber(L_client, 0.0);
			lua_call(L_client, 2, 0);
		} else {
			printf("hook_tick DNE\n");
		}

		if(map_client != NULL)
		{
			map_tick_atmos(map_client);
			for(y = 0; y < map_client->h && y < 12; y++)
			for(x = 0; x < map_client->w && x < 20; x++)
			{
				uint32_t *p;
				int sx, sy;

				cell_t *c = &(map_client->c[y*map_client->w + x]);
				uint32_t v = 0xFFFF00FF;

				switch(c->turf.type)
				{
					case TURF_WATER:
						v = 0xFF0000FF;
						break;
					case TURF_FLOOR:
						v = 0xFFAAAAAA;
						break;
					case TURF_WALL:
						v = 0xFF555555;
						break;

				}
				v = (((int)(c->gas.g.o2*255))<<8) | (((int)(c->gas.g.n2*255))<<16) | 0xFF000000;

				for(sy = 0; sy < 16; sy++)
				{
					p = (uint32_t *)(screen->pixels + (y*16+sy)*screen->pitch);
					p += x*16;
					for(sx = 0; sx < 16; sx++)
					{
						int dtidx = ((sx)&3)|(((sy)&3)<<2);
						dtidx = (dtidx+(bubcount>>3))&15;
						*(p++) = (c->gas.g.water*16.0f - 0.5f > dithtab4[dtidx]
							? 0x000000FF
							: 0x00000000) | v;
					}
				}
			}
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

		bubcount++;

		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_QUIT:
				quitflag = 1;
				break;
		}
	}

	return 0;
}

