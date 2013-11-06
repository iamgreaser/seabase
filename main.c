/*
i'll sort out the licensing crap when i get around to it --GM
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>
#include <time.h>

#include <SDL.h>
//include <enet/enet.h>

#include <squirrel.h>

#include <signal.h>

#define D_E 0x01
#define D_S 0x02
#define D_W 0x04
#define D_N 0x08

typedef struct gasmix
{
	// i'll explain what roles these fulfil.
	float o2; // breathable.
	float n2; // padding.
	float co2; // processed o2.
	float ch4; // poisonous. also, flammable.

	float water; // this is a sea base.
} gasmix_t;

/*
How the turfs work:

TURF_WATER: Nothing but water above and below. Can enter.
TURF_FLOOR: Top and bottom protected. Can enter.
TURF_WALL: Cannot enter. Water/gas cannot flow.
*/
enum
{
	TURF_WATER = 0,
	TURF_FLOOR,
	TURF_WALL,
};

enum
{
	UD_INVALID = 0,

	UD_MAP,
};

typedef struct img
{
	int ud;
	int w, h;
	uint32_t *data;
} img_t;

typedef struct sprite
{
	int ud;
	img_t *img;
	int tx, ty;
	int tw, th;
} sprite_t;

typedef struct turf
{
	uint8_t type;
} turf_t;

typedef struct cell cell_t;
struct cell
{
	gasmix_t gas;
	turf_t turf;
	float vx, vy;
};

typedef struct map
{
	int ud;
	int w, h;
	cell_t c[];
} map_t;

SDL_Surface *screen;
HSQUIRRELVM S_client;
HSQUIRRELVM S_server;
map_t *map_client = NULL;
map_t *map_server = NULL;

/**
	\brief Load a file directly from the filesystem, bypassing filename security.

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
char *file_get_direct(const char *fname, int *len)
{
	FILE *fp = fopen(fname, "rb");

	if(fp == NULL)
	{
		perror("file_get_direct.fopen");
		return NULL;
	}

	int buflen = 1024;
	int bufoffs = 0;
	char *buf = malloc(buflen);

	for(;;)
	{
		int fcount = fread(buf+bufoffs, 1, buflen-bufoffs, fp);

		if(fcount == -1)
		{
			perror("file_get_direct.fread");
			free(buf);
			fclose(fp);
			return NULL;
		}

		if(fcount == buflen-bufoffs)
		{
			bufoffs = buflen;
			buflen = (buflen*3/2+1);

			if(buflen < bufoffs+1)
				buflen = bufoffs+1;

			buf = realloc(buf, buflen);
		} else {
			fcount--; // I don't know why I have to do this, I just do.
			//printf("%i %i\n", bufoffs, fcount);
			*len = bufoffs + fcount;
			buf[*len] = '\x00';
			break;
		}
	}

	fclose(fp);
	//printf("file: [%s]\n", buf);

	return buf;
}

/**
	\brief Load a file using the appropriate method.

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
char *file_get(const char *fname, int *len)
{
	// TODO: filename security and whatnot
	return file_get_direct(fname, len);
}

/**
	\brief Compile a file using file_get.

	\param S Relevant Squirrel virtual machine.
	\param fname Name of file to be read.
	\return Boolean indicating if the operation succeeded.
*/
int hsq_compile(HSQUIRRELVM S, const char *fname)
{
	int blen = 0;
	char *buf = file_get(fname, &blen);

	if(buf == NULL)
		return 0;

	SQRESULT ret = sq_compilebuffer(S, buf, blen, fname, SQTrue);
	free(buf);

	return SQ_SUCCEEDED(ret);
}

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
	\brief SQ: Sets the turf type for a map.

	\param map
	\param x
	\param y
	\param type Turf type as defined by the TURF_* constants.
*/
SQInteger fsq_turf_set_type(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);
	if(top != 5) return SQ_ERROR;

	SQUserPointer map_ud;
	SQInteger x, y, type;
	if(SQ_FAILED(sq_getuserpointer(S, 2, &map_ud))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 3, &x))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 4, &y))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 5, &type))) return SQ_ERROR;

	map_t *map = (map_t *)map_ud;
	if(map->ud != UD_MAP) return SQ_ERROR;
	if(x < 0) return SQ_ERROR;
	if(y < 0) return SQ_ERROR;
	if(x >= map->w) return SQ_ERROR;
	if(y >= map->h) return SQ_ERROR;

	int mi = y*map->w + x;
	map->c[mi].turf.type = type;

	return 0;
}

/**
	\brief SQ: Creates a map.

	\param w Width of map.
	\param h Height of map.

	\return Map userpointer.
*/
SQInteger fsq_map_new(HSQUIRRELVM S)
{
	SQInteger top = sq_gettop(S);
	if(top != 3) return SQ_ERROR;

	SQInteger w, h;
	if(SQ_FAILED(sq_getinteger(S, 2, &w))) return SQ_ERROR;
	if(SQ_FAILED(sq_getinteger(S, 3, &h))) return SQ_ERROR;

	if(w <= 0) return SQ_ERROR;
	if(h <= 0) return SQ_ERROR;

	map_t *map = malloc(sizeof(map_t) + sizeof(cell_t)*w*h);
	map->ud = UD_MAP;
	map->w = w;
	map->h = h;

	int i;
	for(i = 0; i < w*h; i++)
	{
		cell_t *c = &(map->c[i]);
		c->turf.type = TURF_WATER;
		cell_reset_gas(c);
	}

	sq_pushuserpointer(S, map);
	map_client = map;
	return 1;
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
	sq_pop(S_client, 1);

	SDL_WM_SetCaption("Sea Base Omega - 0.0 prealpha", NULL);
	screen = SDL_SetVideoMode(800, 600, 32, 0);

	if(hsq_compile(S_client, "pkg/base/main_client.sq"))
	{
		sq_pushroottable(S_client);
		sq_call(S_client, 1, SQFalse, SQTrue);
		sq_pop(S_client, 1);
	} else {
		printf("File failed to compile\n");
	}

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

		for(sy = 0; sy < 16; sy++)
		{
			p = (uint32_t *)(screen->pixels + (y*16+sy)*screen->pitch);
			p += x*16;
			for(sx = 0; sx < 16; sx++)
				*(p++) = v;
		}
	}
	SDL_Flip(screen);

	SDL_Delay(1000);

	sq_close(S_client);
	sq_close(S_server);

	return 0;
}

