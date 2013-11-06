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
	gasmix_t gas_new;
	turf_t turf;
	float vx, vy;
};

typedef struct map
{
	int ud;
	int w, h;
	cell_t c[];
} map_t;

// file.c

// sq.c
SQInteger fsq_turf_reset_gas(HSQUIRRELVM S);
SQInteger fsq_turf_set_type(HSQUIRRELVM S);
SQInteger fsq_map_new(HSQUIRRELVM S);

// main.c

