/*
i'll sort out the licensing crap when i get around to it --GM
*/

#include "common.h"

SDL_Surface *screen;
lua_State *L_client;
lua_State *L_server;
map_t *map_client = NULL;
map_t *map_server = NULL;

int bubcount = 0;

/**
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

int main(int argc, const char *argv)
{
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	atexit(SDL_Quit);

	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	L_server = luaL_newstate();
	L_client = luaL_newstate();

	// TODO: restrict the libraries loaded to a subset
	printf("Loading functions\n");
	luaL_openlibs(L_server);
	luaL_openlibs(L_client);

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
	screen = SDL_SetVideoMode(800, 600, 32, 0);

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
			for(y = 0; y < map_client->h && y < 18; y++)
			for(x = 0; x < map_client->w && x < 25; x++)
			{
				uint32_t *p;
				int sx, sy;

				cell_t *c = &(map_client->c[y*map_client->h + x]);
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

				for(sy = 0; sy < 32; sy++)
				{
					p = (uint32_t *)(screen->pixels + (y*32+sy)*screen->pitch);
					p += x*32;
					for(sx = 0; sx < 32; sx++)
					{
						int dtidx = ((sx>>1)&3)|(((sy>>1)&3)<<2);
						dtidx = (dtidx+(bubcount>>3))&15;
						*(p++) = (c->gas.g.water*16.0f - 0.5f > dithtab4[dtidx]
							? 0x000000FF
							: 0x00000000) | v;
					}
				}
			}
		}
		SDL_Flip(screen);

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

