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

#define GAS_COUNT 5
typedef union gasmix
{
	struct {
		// i'll explain what roles these fulfil.
		float water; // this is a sea base.

		float o2; // breathable.
		float n2; // padding.
		float co2; // processed o2.
		float ch4; // poisonous. also, flammable.

		float vx, vy; // velocity
	} g;

	float a[GAS_COUNT];
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
	gasmix_t gas_old;
	turf_t turf;
};

typedef struct map
{
	int ud;
	int w, h;
	cell_t c[];
} map_t;

// file.c
char *file_get_direct(const char *fname, int *len);
char *file_get(const char *fname, int *len);
int hsq_compile(HSQUIRRELVM S, const char *fname);

// map.c
void cell_reset_gas(cell_t *c);
void map_tick_atmos(map_t *map);

// sq.c
SQInteger fsq_turf_reset_gas(HSQUIRRELVM S);
SQInteger fsq_turf_set_type(HSQUIRRELVM S);
SQInteger fsq_map_new(HSQUIRRELVM S);

// main.c

