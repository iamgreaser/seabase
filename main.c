/*
i'll sort out the licensing crap when i get around to it --GM
*/

#include "common.h"

SDL_Surface *screen;
HSQUIRRELVM S_client;
HSQUIRRELVM S_server;
map_t *map_client = NULL;
map_t *map_server = NULL;

/**
	\brief Resets the gas for a cell according to the turf type.

	\param c Cell to reset.
*/
void cell_reset_gas(cell_t *c)
{
	c->vx = c->vy = 0.0f;

	c->gas.co2 = 0.0f;
	c->gas.ch4 = 0.0f;
	c->gas.water = 0.0f;
	c->gas.o2 = 0.0f;
	c->gas.n2 = 0.0f;
	switch(c->turf.type)
	{
		case TURF_WATER:
			c->gas.water = 1.0f;
			break;
		case TURF_FLOOR:
			c->gas.o2 = 0.2f;
			c->gas.n2 = 0.8f;
			break;
		default:
			break;
	}
}

/**
	\brief Simple stdout print function for the Squirrel VM.
*/
void hsq_print_stdout(HSQUIRRELVM S, const SQChar *buf, ...)
{
	va_list vl;

	printf((S == S_server ? "[server] " : S == S_client ? "[client] " : "[??????] "));

	va_start(vl, buf);
	vprintf(buf, vl);
	va_end(vl);

	printf("\n");
}

/**
	\brief Simple error handler for the Squirrel VM.
*/
SQInteger hsq_error(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);

	printf("*ERROR* ");
	printf((S == S_server ? "[server] " : S == S_client ? "[client] " : "[??????] "));
	const SQChar *c1 = "FAIL";
	sq_getstring(S, -1, &c1);
	printf("%s\n", c1);

	return 0;
}

int main(int argc, const char *argv)
{
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	atexit(SDL_Quit);

	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	S_server = sq_open(1024);
	S_client = sq_open(1024);

	sq_setprintfunc(S_server, (SQPRINTFUNCTION)hsq_print_stdout);
	sq_setprintfunc(S_client, (SQPRINTFUNCTION)hsq_print_stdout);

	sq_newclosure(S_client, (SQFUNCTION)hsq_error, 0);
	sq_seterrorhandler(S_client);

	sq_pushroottable(S_client);
	sq_pushstring(S_client, "map_new", -1);
	sq_newclosure(S_client, (SQFUNCTION)fsq_map_new, 0);
	sq_newslot(S_client, -3, SQFalse);
	sq_pushstring(S_client, "turf_set_type", -1);
	sq_newclosure(S_client, (SQFUNCTION)fsq_turf_set_type, 0);
	sq_newslot(S_client, -3, SQFalse);
	sq_pushstring(S_client, "turf_reset_gas", -1);
	sq_newclosure(S_client, (SQFUNCTION)fsq_turf_reset_gas, 0);
	sq_newslot(S_client, -3, SQFalse);
	sq_pop(S_client, 1);

	SDL_WM_SetCaption("Sea Base Omega - 0.0 prealpha", NULL);
	screen = SDL_SetVideoMode(800, 600, 32, 0);

	if(hsq_compile(S_client, "pkg/base/main_client.nut"))
	{
		sq_pushroottable(S_client);
		sq_call(S_client, 1, SQFalse, SQTrue);
		sq_pop(S_client, 1);
	} else {
		printf("File failed to compile\n");
	}

	int quitflag = 0;
	while(!quitflag)
	{
		int x, y;
		memset(screen->pixels, 0, screen->pitch*screen->h);

		for(y = 0; y < map_client->h && y < 37; y++)
		for(x = 0; x < map_client->w && x < 50; x++)
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
			v = (((int)(c->gas.o2*255))<<8) | ((int)(c->gas.water*255)) | 0xFF000000;

			for(sy = 0; sy < 16; sy++)
			{
				p = (uint32_t *)(screen->pixels + (y*16+sy)*screen->pitch);
				p += x*16;
				for(sx = 0; sx < 16; sx++)
					*(p++) = v;
			}
		}
		SDL_Flip(screen);

		SDL_Delay(10);

		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_QUIT:
				quitflag = 1;
				break;
		}
	}

	sq_close(S_client);
	sq_close(S_server);

	return 0;
}

