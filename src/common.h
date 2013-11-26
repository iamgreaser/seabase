#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>
#include <time.h>

#include <SDL.h>
//include <enet/enet.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <signal.h>

#include <zlib.h>

// for ntohs/ntohl
#include <netinet/in.h>

#define D_E 0x01
#define D_S 0x02
#define D_W 0x04
#define D_N 0x08

#define BF_M_AMODE  0x00000003
#define BF_AM_DIRECT   0x00000000
#define BF_AM_THRES    0x00000001
#define BF_AM_BLEND    0x00000002
#define BF_AM_DITHER   0x00000003

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

	UD_LOADING, // special type to indicate file is being fetched
	UD_FAILED, // special type to indicate loading failed

	UD_MAP,
	UD_IMG,
};

enum
{
	FMT_INVALID = 0,

	FMT_IMG_RAW,
	FMT_IMG_PNG,
};

typedef struct loading loading_t;
typedef struct ud ud_t;

struct loading
{
	int ud; /// userdata type
	void *v; /// pointer to actual data
	int dlen, alen; /// data length, allocated length

	int fmt; /// file format
	ud_t *p, *n; /// previous/next ud_t blocks in chain
	const char *fname; /// name of file
	int resid; /// resource ID (sanity check)
};

struct ud
{
	int ud; /// userdata type
	void *v; /// pointer to actual data
	int dlen, alen; /// data length, allocated length
};

typedef struct img
{
	int w, h;
	uint32_t data[];
} img_t;

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
	int w, h;
	cell_t c[];
} map_t;

// blit.c
void blit_raw(
	uint32_t *sd, int sw, int sp, int sh, int sx, int sy,
	uint32_t *dd, int dw, int dp, int dh, int dx, int dy,
	int bw, int bh, int flags);
void blit_img_to_img(
	img_t *sd, int sx, int sy,
	img_t *dd, int dx, int dy,
	int bw, int bh, int flags);
void blit_img_to_sdl(
	img_t *sd, int sx, int sy,
	SDL_Surface *dd, int dx, int dy,
	int bw, int bh, int flags);
void blit_sdl_to_img(
	SDL_Surface *sd, int sx, int sy,
	img_t *dd, int dx, int dy,
	int bw, int bh, int flags);
void blit_sdl_to_sdl(
	SDL_Surface *sd, int sx, int sy,
	SDL_Surface *dd, int dx, int dy,
	int bw, int bh, int flags);

// file.c
char *file_get_direct(const char *fname, int *len);
char *file_get(const char *fname, int *len);

// img.c
void img_free(img_t *img);
img_t *img_new(int w, int h);
ud_t *img_provide_ud(lua_State *L, img_t *img);
img_t *img_new_ud(lua_State *L, int w, int h);

// lua.c / lua_*.c
ud_t *ud_get_block(lua_State *L, int typ, char *tname, int idx);
int fl_img_new(lua_State *L);
int fl_img_load(lua_State *L);
int fl_img_blit(lua_State *L);
int fl_map_new(lua_State *L);
int fl_turf_get_gas(lua_State *L);
int fl_turf_set_gas(lua_State *L);
int fl_turf_reset_gas(lua_State *L);
int fl_turf_set_type(lua_State *L);

// map.c
void cell_reset_gas(cell_t *c);
void map_free(map_t *map);
map_t *map_new(int w, int h);
map_t *map_new_ud(lua_State *L, int w, int h);
void map_tick_atmos(map_t *map);

// png.c
img_t *img_load_png(const char *data, int len);

// main.c
extern SDL_Surface *screen;

