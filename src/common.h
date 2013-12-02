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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <SDL.h>
//include <enet/enet.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <signal.h>

#include <zlib.h>

// for ntohs/ntohl
#ifdef WIN32
// Fuck you Windows.
#define ntohs(x) ((uint16_t)((((uint16_t)(x))>>8)|(((uint16_t)(x))<<8)))
#define ntohl(x) ((uint32_t)(((uint32_t)ntohs(((uint32_t)(x))>>16))|(((uint32_t)ntohs(x))<<16)))
#else
#include <netinet/in.h>
#endif

#define D_E 0x01
#define D_S 0x02
#define D_W 0x04
#define D_N 0x08

#define BF_M_AMODE  0x00000003
#define BF_AM_DIRECT   0x00000000
#define BF_AM_THRES    0x00000001
#define BF_AM_BLEND    0x00000002
#define BF_AM_DITHER   0x00000003

enum
{
	SEC_FORBID = 0,
	SEC_LOCAL,
	SEC_REMOTE,
};

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
	UD_ANY, /// special type to indicate we can accept ANY block

	UD_LOADING, /// special type to indicate file is being fetched
	UD_FAILED, /// special type to indicate loading failed

	UD_LUA,

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

	char *fmt; /// file format
	ud_t *p, *n; /// previous/next ud_t blocks in chain
	char *fname; /// name of file
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
int file_sec_check(const char *fname, int is_client, int is_write);
void file_parse_any(lua_State *L, const char *data, int len, const char *fmt, const char *fname);
char *file_get(const char *fname, int *len);

// img.c
void img_free(img_t *img);
img_t *img_new(int w, int h);
ud_t *img_provide_ud(lua_State *L, img_t *img);
img_t *img_new_ud(lua_State *L, int w, int h);

// iiiiinnnnpuuuuuuuuuuuhhhhht.........c
void input_mouse_get_lua(lua_State *L);
void input_mouse_update_pos(lua_State *L, int x, int y);
void input_mouse_update_button(lua_State *L, int x, int y, int b, int state);

// lua.c / lua_*.c
ud_t *ud_get_block(lua_State *L, int typ, char *tname, int idx);
int fl_call_proxy(lua_State *L);
int fl_block(lua_State *L);
int fl_fetch(lua_State *L);
int fl_draw_rect_fill(lua_State *L);
int fl_draw_rect_outl(lua_State *L);
int fl_img_new(lua_State *L);
int fl_img_get_dims(lua_State *L);
int fl_img_get_pixel(lua_State *L);
int fl_img_blit(lua_State *L);
int fl_map_new(lua_State *L);
int fl_map_tick_atmos(lua_State *L);
int fl_mouse_get(lua_State *L);
int fl_turf_get_gas(lua_State *L);
int fl_turf_set_gas(lua_State *L);
int fl_turf_reset_gas(lua_State *L);
int fl_turf_get_type(lua_State *L);
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
void eprintf(const char *fmt, ...);
extern SDL_Surface *screen;
extern int is_client;

